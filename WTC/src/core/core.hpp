#pragma once
#include <mutex>
#include "runtime_context.hpp"

namespace core
{
    enum class runtime_state_t : uint8_t
    {
        Disconnected = 0,
        Connecting,
        Connected,
        InBattle
    };

    inline auto ReloadRuntime() -> bool;
    inline auto RestartRuntime() -> bool;
    inline auto MaintainConnection(bool force = false) -> void;
    inline auto Shutdown() -> void;
    inline auto IsConnected() -> bool;
    inline auto GetRuntimeState() -> runtime_state_t;

    inline std::atomic<bool> runtime_connected = false;
    inline std::atomic<bool> workers_started = false;
    inline std::atomic<bool> was_in_battle = false;
    inline std::atomic<bool> reconnect_in_progress = false;
    inline std::atomic<ULONGLONG> next_reconnect_attempt_tick = 0;
    inline std::atomic<ULONGLONG> next_battle_state_poll_tick = 0;
    inline std::atomic<runtime_state_t> runtime_state = runtime_state_t::Disconnected;
    inline std::atomic<int> entity_update_interval_ms = 100;
    inline std::atomic<int> frame_update_interval_ms = 8;
    inline std::atomic<int> projectile_update_interval_ms = 8;
    inline std::jthread entity_worker = {};
    inline std::jthread frame_worker = {};
    inline std::jthread projectile_worker = {};
    inline std::mutex reconnect_mutex = {};

    inline auto set_runtime_state(runtime_state_t state, const char* reason = nullptr) -> void
    {
        const runtime_state_t prev = runtime_state.exchange(state, std::memory_order_acq_rel);
        if (prev == state)
            return;

        if (reason && reason[0] != '\0')
            LOG("[STATE] %d -> %d (%s)\n", static_cast<int>(prev), static_cast<int>(state), reason);
        else
            LOG("[STATE] %d -> %d\n", static_cast<int>(prev), static_cast<int>(state));
    }

    template <typename Fn>
    inline void run_periodic_loop(std::stop_token stop_token, std::atomic<int>& interval_ms, Fn&& fn)
    {
        auto next_tick = std::chrono::steady_clock::now();
        while (!stop_token.stop_requested())
        {
            const int interval_value = std::clamp(interval_ms.load(std::memory_order_relaxed), 1, 200);
            const auto period = std::chrono::milliseconds(interval_value);
            fn();
            if (stop_token.stop_requested())
                break;

            next_tick += period;
            std::this_thread::sleep_until(next_tick);

            const auto now = std::chrono::steady_clock::now();
            if (now - next_tick > period)
                next_tick = now;
        }
    }

    inline auto Thread() -> bool
    {
        if (workers_started.exchange(true, std::memory_order_acq_rel))
            return true;

        MaintainConnection(true);

        entity_worker = std::jthread([](std::stop_token stop_token)
        {
            run_periodic_loop(stop_token, entity_update_interval_ms, []()
            {
                if (!IsConnected())
                    return;
                misc::UpdateEntityList();
            });
        });

        // Frame snapshot is refreshed in render tick for tighter camera/entity coherence.

        projectile_worker = std::jthread([](std::stop_token stop_token)
        {
            run_periodic_loop(stop_token, projectile_update_interval_ms, []()
            {
                if (!IsConnected())
                    return;
                misc::UpdateProjectileList();
            });
        });

        return true;
    }

    inline auto ReloadRuntime() -> bool
    {
        misc::reset_runtime_cache();

        const uint64_t base_address = TargetProcess->GetBaseAddress("aces.exe");
        const uint64_t base_size = TargetProcess->GetBaseSize("aces.exe");
        runtime::set_base_info(base_address, base_size);

        if (!base_address || !base_size)
        {
            LOG("Failed to refresh base information.\n");
            return false;
        }

        offsets::initialize_external_offsets();

        if (!update::run())
        {
            LOG("Failed to update sdk.\n");
            return false;
        }

        if (!sdk::init())
        {
            LOG("Failed to initialize sdk.\n");
            return false;
        }

        return true;
    }

    inline auto RestartRuntime() -> bool
    {
        std::scoped_lock lock(reconnect_mutex);
        runtime_connected.store(false);
        was_in_battle.store(false);
        next_reconnect_attempt_tick.store(0);
        next_battle_state_poll_tick.store(0);
        set_runtime_state(runtime_state_t::Disconnected, "manual restart");
        misc::reset_runtime_cache();
        TargetProcess->Disconnect();
        sdk::manual_reload_status = "Reconnecting...";

        MaintainConnection(true);
        return runtime_connected.load();
    }

    inline auto MaintainConnection(bool force) -> void
    {
        bool expected = false;
        if (!reconnect_in_progress.compare_exchange_strong(expected, true))
            return;
        struct reconnect_flag_guard_t
        {
            ~reconnect_flag_guard_t()
            {
                reconnect_in_progress.store(false);
            }
        } reconnect_guard;

        const bool connected_now = runtime_connected.load(std::memory_order_acquire);
        const bool should_attempt_reconnect = force || !connected_now;
        if (should_attempt_reconnect)
        {
            const ULONGLONG now = GetTickCount64();
            if (!force)
            {
                const ULONGLONG next_tick = next_reconnect_attempt_tick.load(std::memory_order_relaxed);
                if (now < next_tick)
                    return;
            }

            next_reconnect_attempt_tick.store(now + 5000, std::memory_order_relaxed);
            set_runtime_state(runtime_state_t::Connecting, force ? "force reconnect" : "periodic reconnect");

            const bool init_ok = TargetProcess->Init("aces.exe");
            if (!init_ok)
            {
                if (runtime_connected.exchange(false))
                    LOG("[!] Lost connection to aces.exe. Waiting for reconnect...\n");
                else
                    LOG("[*] aces.exe not found. Retry in 5 seconds...\n");

                was_in_battle.store(false);
                next_battle_state_poll_tick.store(0, std::memory_order_relaxed);
                misc::reset_runtime_cache();
                sdk::manual_reload_status = "Waiting for aces.exe...";
                set_runtime_state(runtime_state_t::Disconnected, "target not found");
                return;
            }

            if (!ReloadRuntime())
            {
                runtime_connected.store(false);
                next_battle_state_poll_tick.store(0, std::memory_order_relaxed);
                misc::reset_runtime_cache();
                sdk::manual_reload_status = "Reconnect failed";
                set_runtime_state(runtime_state_t::Disconnected, "reload failed");
                return;
            }

            runtime_connected.store(true, std::memory_order_release);
            sdk::manual_reload_status = "Connected";
            set_runtime_state(runtime_state_t::Connected, "connected");
            LOG("[+] Connected to aces.exe\n");
        }

        const ULONGLONG now_poll = GetTickCount64();
        const ULONGLONG next_poll_tick = next_battle_state_poll_tick.load(std::memory_order_relaxed);
        if (now_poll < next_poll_tick)
            return;

        next_battle_state_poll_tick.store(now_poll + 250, std::memory_order_relaxed);

        if (!sdk::cLocalPlayer)
        {
            was_in_battle.store(false, std::memory_order_relaxed);
            set_runtime_state(runtime_state_t::Connected, "local player unavailable");
            return;
        }

        const auto gui_state_raw = sdk::cLocalPlayer->getGuiState();
        const auto gui_state = static_cast<GuiState>(gui_state_raw);
        const bool in_battle = (gui_state == GuiState::ALIVE || gui_state == GuiState::SPEC);
        const bool was_battle_before = was_in_battle.exchange(in_battle);

        if (in_battle && !was_battle_before)
        {
            LOG("[*] Entered battle. Refreshing runtime...\n");
            if (!ReloadRuntime())
            {
                runtime_connected.store(false);
                was_in_battle.store(false);
                misc::reset_runtime_cache();
                sdk::manual_reload_status = "Battle reload failed";
                set_runtime_state(runtime_state_t::Disconnected, "battle reload failed");
                return;
            }

            sdk::manual_reload_status = "Battle reloaded";
            set_runtime_state(runtime_state_t::InBattle, "entered battle");
        }
        else if (!in_battle)
        {
            set_runtime_state(runtime_state_t::Connected, "left battle");
        }
    }

    inline auto IsConnected() -> bool
    {
        return runtime_connected.load();
    }

    inline auto GetRuntimeState() -> runtime_state_t
    {
        return runtime_state.load(std::memory_order_acquire);
    }

    inline auto Shutdown() -> void
    {
        if (!workers_started.exchange(false))
            return;

        if (entity_worker.joinable())
            entity_worker.request_stop();
        if (frame_worker.joinable())
            frame_worker.request_stop();
        if (projectile_worker.joinable())
            projectile_worker.request_stop();

        if (entity_worker.joinable())
            entity_worker.join();
        if (frame_worker.joinable())
            frame_worker.join();
        if (projectile_worker.joinable())
            projectile_worker.join();
    }
}
