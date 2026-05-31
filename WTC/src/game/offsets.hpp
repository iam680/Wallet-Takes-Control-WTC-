#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

// === namespace offsets ===
namespace offsets
{
    inline uintptr_t alllistdata_offset = 0x6eca7f0;
    inline uintptr_t bomb_index = 0x6c92268;
    inline uintptr_t ccrp_switch_offset = 0x6CB5C98;
    inline uintptr_t cgame_offset = 0x6cb2f68;
    inline uintptr_t hud_offset = 0x6cb2550;
    inline uintptr_t localplayer_offset = 0x6c8f758;
    inline uintptr_t rocket_index = 0x6c936b0;
    inline uintptr_t yaw_offset = 0x6cb64a8;

    inline uintptr_t is_scoping_offset = 0x6CF7A2C;
    inline uintptr_t screen_width_offset = 0x6ED0080;
    inline uintptr_t view_matrix_offset = 0x6D08C38;

    namespace cgame_offsets
    {
        inline uintptr_t ballistics_offset = 0x3f0;
        inline uintptr_t camera_offset = 0x670;
        inline uintptr_t current_map = 0x1F0;
        inline uintptr_t unit_count1 = 0x320;
        inline uintptr_t unit_count2 = 0x338;
        inline uintptr_t unit_count3 = 0x350;
        inline uintptr_t unit_list1 = 0x310;
        inline uintptr_t unit_list2 = 0x328;
        inline uintptr_t unit_list3 = 0x340;

        namespace camera_offsets
        {
            inline uintptr_t camera_matrix_offset = 0x1d0;
            inline uintptr_t camera_position_offset = 0x7a0;
        }

        namespace ballistic_offsets
        {
            inline uintptr_t bomb_prediction_offset = 0x1c1c;
            inline uintptr_t round_caliber_offset = 0x2060;
            inline uintptr_t round_length_offset = 0x2064;
            inline uintptr_t round_mass_offset = 0x205c;
            inline uintptr_t round_velocity_offset = 0x2050;
            inline uintptr_t telecontrol_offset = 0xcc0;
            inline uintptr_t max_dist = 0x0;

            // Aliases for backward compatibility with existing code
            inline uintptr_t& velocity = round_velocity_offset;
            inline uintptr_t& mass = round_mass_offset;
            inline uintptr_t& caliber = round_caliber_offset;
            inline uintptr_t& length = round_length_offset;
            inline uintptr_t& bullet_impact_point = bomb_prediction_offset;
            inline uintptr_t& bomb_impact_point = bomb_prediction_offset;

            namespace telecontrol_offsets
            {
                inline uintptr_t gameui_offset = 0x7e8;
            }
        }
    }

    namespace localplayer
    {
        inline uintptr_t brokenSlots_offset = 0x570;
        inline uintptr_t cachedIsAutoSquad_offset = 0xb90;
        inline uintptr_t clanTag_offset = 0x188;
        inline uintptr_t classinessMark_offset = 0x610;
        inline uintptr_t controlledUnitRef_offset = 0x918;
        inline uintptr_t countryId_offset = 0x250;
        inline uintptr_t customState_offset = 0x2b0;
        inline uintptr_t decals_offset = 0x200;
        inline uintptr_t nik_offset = 0x78;
        inline uintptr_t disabledByMatchingSlots_offset = 0x540;
        inline uintptr_t dummyForCountUsedSlots_offset = 0x750;
        inline uintptr_t dummyForCrewUnitsList_offset = 0x510;
        inline uintptr_t dummyForFootballStat_offset = 0xaf0;
        inline uintptr_t dummyForKillStreaksProgress_offset = 0x860;
        inline uintptr_t dummyForPlayerStat_offset = 0xac0;
        inline uintptr_t dummyForRoundScore_offset = 0xa60;
        inline uintptr_t dummyForSpawnCosts_offset = 0x790;
        inline uintptr_t dummyForSpawnDelayTimes_offset = 0x7d0;
        inline uintptr_t dummyForSupportPlanes_offset = 0x440;
        inline uintptr_t forceLockTarget_offset = 0xb68;
        inline uintptr_t forcedRespawn_offset = 0x690;
        inline uintptr_t guiState_offset = 0x6e0;
        inline uintptr_t invitedNickName_offset = 0x138;
        inline uintptr_t localunit_offset = 0x8e8;
        inline uintptr_t memberId_offset = 0x278;
        inline uintptr_t missionSupportUnitEnabled_offset = 0xc10;
        inline uintptr_t missionSupportUnitRef_offset = 0xbe0;
        inline uintptr_t nickFrame_offset = 0xbb8;
        inline uintptr_t nickLocKey_offset = 0x160;
        inline uintptr_t numFreeSpareUsed_offset = 0xc60;
        inline uintptr_t ownedSlots_offset = 0x5e8;
        inline uintptr_t ownedUnitRef_offset = 0x8e8;
        inline uintptr_t publicFlags_offset = 0x1d8;
        inline uintptr_t rageTokens_offset = 0xc38;
        inline uintptr_t realNick_offset = 0xb18;
        inline uintptr_t soldier_offset = 0x3228;
        inline uintptr_t spareAircraftInSlots_offset = 0x5c0;
        inline uintptr_t spectatedModel_offset = 0x708;
        inline uintptr_t squadScore_offset = 0x890;
        inline uintptr_t squadronId_offset = 0xb40;
        inline uintptr_t stateRefl_offset = 0x818;
        inline uintptr_t supportUnitRef_offset = 0x958;
        inline uintptr_t timeToKick_offset = 0x6b8;
        inline uintptr_t timeToRespawnInCoop_offset = 0x668;
        inline uintptr_t timeToRespawn_offset = 0x640;
        inline uintptr_t wasReadySlots_offset = 0x598;
        inline uintptr_t wreckedPartShipUnitRef_offset = 0x988;
        inline uintptr_t nikename = 0xB8;
    }

    namespace unit_offsets
    {
        inline uintptr_t a_7k_bomb_mk81_snakeye_offset = 0x7e0;
        inline uintptr_t airmovement_offset = 0xd18;
        inline uintptr_t allowBailout_offset = 0xca0;
        inline uintptr_t armada_01_0_offset = 0x1020;
        inline uintptr_t bbmax_offset = 0x24c;
        inline uintptr_t bbmin_offset = 0x240;
        inline uintptr_t bombDelayExplosion_offset = 0xb08;
        inline uintptr_t briefMalfunctionState_offset = 0xae0;
        inline uintptr_t brokenPropFlags_offset = 0x3318;
        inline uintptr_t brokenTurretDriveJammedTime_offset = 0x820;
        inline uintptr_t brokenTurretDriveMult_offset = 0x870;
        inline uintptr_t brokenTurretDriveSpeed_offset = 0x848;
        inline uintptr_t crewStatMult_offset = 0x1b20;
        inline uintptr_t curNightVisionMode_offset = 0x930;
        inline uintptr_t default_offset = 0x1fe0;
        inline uintptr_t delayWithFlightTime_offset = 0xb30;
        inline uintptr_t dummyForDeathInfo_offset = 0xbb8;
        inline uintptr_t dummyForUnitFlags_offset = 0xf10;
        inline uintptr_t dummyVarForCrewLayout_offset = 0x960;
        inline uintptr_t dummyVarForMissionAddText_offset = 0x988;
        inline uintptr_t enableCiwsGunners_offset = 0xeb0;
        inline uintptr_t enableGunners_offset = 0xe88;
        inline uintptr_t extinguishAssistant_offset = 0xde8;
        inline uintptr_t extinguishAssistee_offset = 0xdb8;
        inline uintptr_t farthestExitZoneId_offset = 0xfa0;
        inline uintptr_t fpvRocketCameraControlEnabled_offset = 0x1110;
        inline uintptr_t fuseModeIndex_offset = 0x1a58;
        inline uintptr_t groundmovement_offset = 0x1f20;
        inline uintptr_t hasModuleEffectsToRepair_offset = 0x8c0;
        inline uintptr_t info_offset = 0xff0;
        inline uintptr_t invulnerable_offset = 0xe60;
        inline uintptr_t isAlternativeShotFreq_offset = 0x1e8;
        inline uintptr_t isBreechDamaged_offset = 0x898;
        inline uintptr_t isNeedExtinguishHelp_offset = 0x1978;
        inline uintptr_t isNeedRepairHelp_offset = 0x1948;
        inline uintptr_t isOnAirfieldAndCanRearmedVal_offset = 0x39e0;
        inline uintptr_t killer_offset = 0x1800;
        inline uintptr_t lastBuildingTime_offset = 0xa90;
        inline uintptr_t lastContactFrom_offset = 0xbf0;
        inline uintptr_t lastContactTimeRel_offset = 0xc78;
        inline uintptr_t lastContactTime_offset = 0xc50;
        inline uintptr_t lastContactTo_offset = 0xc20;
        inline uintptr_t lowRateUnitFlags_offset = 0xf38;
        inline uintptr_t maskingFactor_offset = 0x19f8;
        inline uintptr_t moduleEffectsRepairAtTime_offset = 0x8e8;
        inline uintptr_t nextUseArtilleryTime_offset = 0x16e0;
        inline uintptr_t persistentAerobaticsSmokeId_offset = 0x32f0;
        inline uintptr_t persistentAerobaticsSmoke_offset = 0x32c8;
        inline uintptr_t pilot_armada_01_0_offset = 0x1de8;
        inline uintptr_t position_offset = 0xd08;
        inline uintptr_t prepareExtinguishAssistantTime_offset = 0x1670;
        inline uintptr_t prepareRepairAssistantTime_offset = 0x1648;
        inline uintptr_t prepareRepairAssisteeTime_offset = 0x1620;
        inline uintptr_t prepareRepairCooldownsTime_offset = 0x17a0;
        inline uintptr_t repairAssistant_offset = 0xd88;
        inline uintptr_t repairAssistee_offset = 0xd58;
        inline uintptr_t repairCooldowns_offset = 0x19a8;
        inline uintptr_t repairTimer_offset = 0x1ed0;
        inline uintptr_t rocketFuseDist_offset = 0xb58;
        inline uintptr_t rotation_matrix_offset = 0xce4;
        inline uintptr_t scoutCooldown_offset = 0x1300;
        inline uintptr_t scoutStartTime_offset = 0x12d8;
        inline uintptr_t smokeScreenActived_offset = 0x1af8;
        inline uintptr_t smokeScreenCount_offset = 0x1ad0;
        inline uintptr_t stealthArmyMask_offset = 0x1068;
        inline uintptr_t stealthRadiusSq_offset = 0x1040;
        inline uintptr_t supportPlaneCatapultsFuseMask_offset = 0xa58;
        inline uintptr_t supportPlanesCount_offset = 0xa30;
        inline uintptr_t timeRepair_offset = 0x1ea8;
        inline uintptr_t timeToNextSmokeScreen_offset = 0x1aa8;
        inline uintptr_t timeToRearm_offset = 0x1ef8;
        inline uintptr_t torpedoDiveDepth_offset = 0xb80;
        inline uintptr_t turret_offset = 0x1098;
        inline uintptr_t unitArmyNo_offset = 0xfe0;
        inline uintptr_t unitState_offset = 0xf60;
        inline uintptr_t visualReloadProgress_offset = 0xab8;
        inline uintptr_t vulnerableByUnitUId_offset = 0xee0;
        inline uintptr_t wingBreakRnd_offset = 0x3340;
        //inline uintptr_t player_ptr_offset = 0x0D78; //íå àêòóàë
        inline uintptr_t bodyBBMax_offset = 0x1F8C;
        inline uintptr_t bodyBBMin_offset = 0x1F80;
        inline uintptr_t damageModelCont_offset = 0x1070;
        inline uintptr_t invulTimer_offset = 0xE3C;
        inline uintptr_t playerInfo_offset = 0xF68;
        inline uintptr_t unitIndex_offset = 0x8;
        inline uintptr_t unitType_offset = 0x8C;
        
        // Alias for backward compatibility
        inline uintptr_t ground_velocity_offset = 0x1f20;
        
        namespace airmovement_offsets
        {
            inline uintptr_t velocity_offset = 0xd18;
        }
    }

    inline bool use_external_offsets_file = false;

    struct runtime_offset_entry
    {
        const char* key;
        uintptr_t* value;
    };

    inline auto runtime_offsets_registry() -> const std::array<runtime_offset_entry, 165>&
    {
        static const std::array<runtime_offset_entry, 165> registry{{
            {"alllistdata_offset", &offsets::alllistdata_offset},
            {"bomb_index", &offsets::bomb_index},
            {"ccrp_switch_offset", &offsets::ccrp_switch_offset},
            {"cgame_offset", &offsets::cgame_offset},
            {"hud_offset", &offsets::hud_offset},
            {"localplayer_offset", &offsets::localplayer_offset},
            {"rocket_index", &offsets::rocket_index},
            {"yaw_offset", &offsets::yaw_offset},
            {"is_scoping_offset", &offsets::is_scoping_offset},
            {"screen_width_offset", &offsets::screen_width_offset},
            {"view_matrix_offset", &offsets::view_matrix_offset},
            {"cgame_offsets.ballistics_offset", &offsets::cgame_offsets::ballistics_offset},
            {"cgame_offsets.camera_offset", &offsets::cgame_offsets::camera_offset},
            {"cgame_offsets.current_map", &offsets::cgame_offsets::current_map},
            {"cgame_offsets.unit_count1", &offsets::cgame_offsets::unit_count1},
            {"cgame_offsets.unit_count2", &offsets::cgame_offsets::unit_count2},
            {"cgame_offsets.unit_count3", &offsets::cgame_offsets::unit_count3},
            {"cgame_offsets.unit_list1", &offsets::cgame_offsets::unit_list1},
            {"cgame_offsets.unit_list2", &offsets::cgame_offsets::unit_list2},
            {"cgame_offsets.unit_list3", &offsets::cgame_offsets::unit_list3},
            {"cgame_offsets.camera_offsets.camera_matrix_offset", &offsets::cgame_offsets::camera_offsets::camera_matrix_offset},
            {"cgame_offsets.camera_offsets.camera_position_offset", &offsets::cgame_offsets::camera_offsets::camera_position_offset},
            {"cgame_offsets.ballistic_offsets.bomb_prediction_offset", &offsets::cgame_offsets::ballistic_offsets::bomb_prediction_offset},
            {"cgame_offsets.ballistic_offsets.round_caliber_offset", &offsets::cgame_offsets::ballistic_offsets::round_caliber_offset},
            {"cgame_offsets.ballistic_offsets.round_length_offset", &offsets::cgame_offsets::ballistic_offsets::round_length_offset},
            {"cgame_offsets.ballistic_offsets.round_mass_offset", &offsets::cgame_offsets::ballistic_offsets::round_mass_offset},
            {"cgame_offsets.ballistic_offsets.round_velocity_offset", &offsets::cgame_offsets::ballistic_offsets::round_velocity_offset},
            {"cgame_offsets.ballistic_offsets.telecontrol_offset", &offsets::cgame_offsets::ballistic_offsets::telecontrol_offset},
            {"cgame_offsets.ballistic_offsets.max_dist", &offsets::cgame_offsets::ballistic_offsets::max_dist},
            {"cgame_offsets.ballistic_offsets.telecontrol_offsets.gameui_offset", &offsets::cgame_offsets::ballistic_offsets::telecontrol_offsets::gameui_offset},
            {"localplayer.brokenSlots_offset", &offsets::localplayer::brokenSlots_offset},
            {"localplayer.cachedIsAutoSquad_offset", &offsets::localplayer::cachedIsAutoSquad_offset},
            {"localplayer.clanTag_offset", &offsets::localplayer::clanTag_offset},
            {"localplayer.classinessMark_offset", &offsets::localplayer::classinessMark_offset},
            {"localplayer.controlledUnitRef_offset", &offsets::localplayer::controlledUnitRef_offset},
            {"localplayer.countryId_offset", &offsets::localplayer::countryId_offset},
            {"localplayer.customState_offset", &offsets::localplayer::customState_offset},
            {"localplayer.decals_offset", &offsets::localplayer::decals_offset},
            {"localplayer.nik_offset", &offsets::localplayer::nik_offset},
            {"localplayer.disabledByMatchingSlots_offset", &offsets::localplayer::disabledByMatchingSlots_offset},
            {"localplayer.dummyForCountUsedSlots_offset", &offsets::localplayer::dummyForCountUsedSlots_offset},
            {"localplayer.dummyForCrewUnitsList_offset", &offsets::localplayer::dummyForCrewUnitsList_offset},
            {"localplayer.dummyForFootballStat_offset", &offsets::localplayer::dummyForFootballStat_offset},
            {"localplayer.dummyForKillStreaksProgress_offset", &offsets::localplayer::dummyForKillStreaksProgress_offset},
            {"localplayer.dummyForPlayerStat_offset", &offsets::localplayer::dummyForPlayerStat_offset},
            {"localplayer.dummyForRoundScore_offset", &offsets::localplayer::dummyForRoundScore_offset},
            {"localplayer.dummyForSpawnCosts_offset", &offsets::localplayer::dummyForSpawnCosts_offset},
            {"localplayer.dummyForSpawnDelayTimes_offset", &offsets::localplayer::dummyForSpawnDelayTimes_offset},
            {"localplayer.dummyForSupportPlanes_offset", &offsets::localplayer::dummyForSupportPlanes_offset},
            {"localplayer.forceLockTarget_offset", &offsets::localplayer::forceLockTarget_offset},
            {"localplayer.forcedRespawn_offset", &offsets::localplayer::forcedRespawn_offset},
            {"localplayer.guiState_offset", &offsets::localplayer::guiState_offset},
            {"localplayer.invitedNickName_offset", &offsets::localplayer::invitedNickName_offset},
            {"localplayer.localunit_offset", &offsets::localplayer::localunit_offset},
            {"localplayer.memberId_offset", &offsets::localplayer::memberId_offset},
            {"localplayer.missionSupportUnitEnabled_offset", &offsets::localplayer::missionSupportUnitEnabled_offset},
            {"localplayer.missionSupportUnitRef_offset", &offsets::localplayer::missionSupportUnitRef_offset},
            {"localplayer.nickFrame_offset", &offsets::localplayer::nickFrame_offset},
            {"localplayer.nickLocKey_offset", &offsets::localplayer::nickLocKey_offset},
            {"localplayer.numFreeSpareUsed_offset", &offsets::localplayer::numFreeSpareUsed_offset},
            {"localplayer.ownedSlots_offset", &offsets::localplayer::ownedSlots_offset},
            {"localplayer.ownedUnitRef_offset", &offsets::localplayer::ownedUnitRef_offset},
            {"localplayer.publicFlags_offset", &offsets::localplayer::publicFlags_offset},
            {"localplayer.rageTokens_offset", &offsets::localplayer::rageTokens_offset},
            {"localplayer.realNick_offset", &offsets::localplayer::realNick_offset},
            {"localplayer.soldier_offset", &offsets::localplayer::soldier_offset},
            {"localplayer.spareAircraftInSlots_offset", &offsets::localplayer::spareAircraftInSlots_offset},
            {"localplayer.spectatedModel_offset", &offsets::localplayer::spectatedModel_offset},
            {"localplayer.squadScore_offset", &offsets::localplayer::squadScore_offset},
            {"localplayer.squadronId_offset", &offsets::localplayer::squadronId_offset},
            {"localplayer.stateRefl_offset", &offsets::localplayer::stateRefl_offset},
            {"localplayer.supportUnitRef_offset", &offsets::localplayer::supportUnitRef_offset},
            {"localplayer.timeToKick_offset", &offsets::localplayer::timeToKick_offset},
            {"localplayer.timeToRespawnInCoop_offset", &offsets::localplayer::timeToRespawnInCoop_offset},
            {"localplayer.timeToRespawn_offset", &offsets::localplayer::timeToRespawn_offset},
            {"localplayer.wasReadySlots_offset", &offsets::localplayer::wasReadySlots_offset},
            {"localplayer.wreckedPartShipUnitRef_offset", &offsets::localplayer::wreckedPartShipUnitRef_offset},
            {"localplayer.nikename", &offsets::localplayer::nikename},
            {"unit_offsets.a_7k_bomb_mk81_snakeye_offset", &offsets::unit_offsets::a_7k_bomb_mk81_snakeye_offset},
            {"unit_offsets.airmovement_offset", &offsets::unit_offsets::airmovement_offset},
            {"unit_offsets.allowBailout_offset", &offsets::unit_offsets::allowBailout_offset},
            {"unit_offsets.armada_01_0_offset", &offsets::unit_offsets::armada_01_0_offset},
            {"unit_offsets.bbmax_offset", &offsets::unit_offsets::bbmax_offset},
            {"unit_offsets.bbmin_offset", &offsets::unit_offsets::bbmin_offset},
            {"unit_offsets.bombDelayExplosion_offset", &offsets::unit_offsets::bombDelayExplosion_offset},
            {"unit_offsets.briefMalfunctionState_offset", &offsets::unit_offsets::briefMalfunctionState_offset},
            {"unit_offsets.brokenPropFlags_offset", &offsets::unit_offsets::brokenPropFlags_offset},
            {"unit_offsets.brokenTurretDriveJammedTime_offset", &offsets::unit_offsets::brokenTurretDriveJammedTime_offset},
            {"unit_offsets.brokenTurretDriveMult_offset", &offsets::unit_offsets::brokenTurretDriveMult_offset},
            {"unit_offsets.brokenTurretDriveSpeed_offset", &offsets::unit_offsets::brokenTurretDriveSpeed_offset},
            {"unit_offsets.crewStatMult_offset", &offsets::unit_offsets::crewStatMult_offset},
            {"unit_offsets.curNightVisionMode_offset", &offsets::unit_offsets::curNightVisionMode_offset},
            {"unit_offsets.default_offset", &offsets::unit_offsets::default_offset},
            {"unit_offsets.delayWithFlightTime_offset", &offsets::unit_offsets::delayWithFlightTime_offset},
            {"unit_offsets.dummyForDeathInfo_offset", &offsets::unit_offsets::dummyForDeathInfo_offset},
            {"unit_offsets.dummyForUnitFlags_offset", &offsets::unit_offsets::dummyForUnitFlags_offset},
            {"unit_offsets.dummyVarForCrewLayout_offset", &offsets::unit_offsets::dummyVarForCrewLayout_offset},
            {"unit_offsets.dummyVarForMissionAddText_offset", &offsets::unit_offsets::dummyVarForMissionAddText_offset},
            {"unit_offsets.enableCiwsGunners_offset", &offsets::unit_offsets::enableCiwsGunners_offset},
            {"unit_offsets.enableGunners_offset", &offsets::unit_offsets::enableGunners_offset},
            {"unit_offsets.extinguishAssistant_offset", &offsets::unit_offsets::extinguishAssistant_offset},
            {"unit_offsets.extinguishAssistee_offset", &offsets::unit_offsets::extinguishAssistee_offset},
            {"unit_offsets.farthestExitZoneId_offset", &offsets::unit_offsets::farthestExitZoneId_offset},
            {"unit_offsets.fpvRocketCameraControlEnabled_offset", &offsets::unit_offsets::fpvRocketCameraControlEnabled_offset},
            {"unit_offsets.fuseModeIndex_offset", &offsets::unit_offsets::fuseModeIndex_offset},
            {"unit_offsets.groundmovement_offset", &offsets::unit_offsets::groundmovement_offset},
            {"unit_offsets.hasModuleEffectsToRepair_offset", &offsets::unit_offsets::hasModuleEffectsToRepair_offset},
            {"unit_offsets.info_offset", &offsets::unit_offsets::info_offset},
            {"unit_offsets.invulnerable_offset", &offsets::unit_offsets::invulnerable_offset},
            {"unit_offsets.isAlternativeShotFreq_offset", &offsets::unit_offsets::isAlternativeShotFreq_offset},
            {"unit_offsets.isBreechDamaged_offset", &offsets::unit_offsets::isBreechDamaged_offset},
            {"unit_offsets.isNeedExtinguishHelp_offset", &offsets::unit_offsets::isNeedExtinguishHelp_offset},
            {"unit_offsets.isNeedRepairHelp_offset", &offsets::unit_offsets::isNeedRepairHelp_offset},
            {"unit_offsets.isOnAirfieldAndCanRearmedVal_offset", &offsets::unit_offsets::isOnAirfieldAndCanRearmedVal_offset},
            {"unit_offsets.killer_offset", &offsets::unit_offsets::killer_offset},
            {"unit_offsets.lastBuildingTime_offset", &offsets::unit_offsets::lastBuildingTime_offset},
            {"unit_offsets.lastContactFrom_offset", &offsets::unit_offsets::lastContactFrom_offset},
            {"unit_offsets.lastContactTimeRel_offset", &offsets::unit_offsets::lastContactTimeRel_offset},
            {"unit_offsets.lastContactTime_offset", &offsets::unit_offsets::lastContactTime_offset},
            {"unit_offsets.lastContactTo_offset", &offsets::unit_offsets::lastContactTo_offset},
            {"unit_offsets.lowRateUnitFlags_offset", &offsets::unit_offsets::lowRateUnitFlags_offset},
            {"unit_offsets.maskingFactor_offset", &offsets::unit_offsets::maskingFactor_offset},
            {"unit_offsets.moduleEffectsRepairAtTime_offset", &offsets::unit_offsets::moduleEffectsRepairAtTime_offset},
            {"unit_offsets.nextUseArtilleryTime_offset", &offsets::unit_offsets::nextUseArtilleryTime_offset},
            {"unit_offsets.persistentAerobaticsSmokeId_offset", &offsets::unit_offsets::persistentAerobaticsSmokeId_offset},
            {"unit_offsets.persistentAerobaticsSmoke_offset", &offsets::unit_offsets::persistentAerobaticsSmoke_offset},
            {"unit_offsets.pilot_armada_01_0_offset", &offsets::unit_offsets::pilot_armada_01_0_offset},
            {"unit_offsets.position_offset", &offsets::unit_offsets::position_offset},
            {"unit_offsets.prepareExtinguishAssistantTime_offset", &offsets::unit_offsets::prepareExtinguishAssistantTime_offset},
            {"unit_offsets.prepareRepairAssistantTime_offset", &offsets::unit_offsets::prepareRepairAssistantTime_offset},
            {"unit_offsets.prepareRepairAssisteeTime_offset", &offsets::unit_offsets::prepareRepairAssisteeTime_offset},
            {"unit_offsets.prepareRepairCooldownsTime_offset", &offsets::unit_offsets::prepareRepairCooldownsTime_offset},
            {"unit_offsets.repairAssistant_offset", &offsets::unit_offsets::repairAssistant_offset},
            {"unit_offsets.repairAssistee_offset", &offsets::unit_offsets::repairAssistee_offset},
            {"unit_offsets.repairCooldowns_offset", &offsets::unit_offsets::repairCooldowns_offset},
            {"unit_offsets.repairTimer_offset", &offsets::unit_offsets::repairTimer_offset},
            {"unit_offsets.rocketFuseDist_offset", &offsets::unit_offsets::rocketFuseDist_offset},
            {"unit_offsets.rotation_matrix_offset", &offsets::unit_offsets::rotation_matrix_offset},
            {"unit_offsets.scoutCooldown_offset", &offsets::unit_offsets::scoutCooldown_offset},
            {"unit_offsets.scoutStartTime_offset", &offsets::unit_offsets::scoutStartTime_offset},
            {"unit_offsets.smokeScreenActived_offset", &offsets::unit_offsets::smokeScreenActived_offset},
            {"unit_offsets.smokeScreenCount_offset", &offsets::unit_offsets::smokeScreenCount_offset},
            {"unit_offsets.stealthArmyMask_offset", &offsets::unit_offsets::stealthArmyMask_offset},
            {"unit_offsets.stealthRadiusSq_offset", &offsets::unit_offsets::stealthRadiusSq_offset},
            {"unit_offsets.supportPlaneCatapultsFuseMask_offset", &offsets::unit_offsets::supportPlaneCatapultsFuseMask_offset},
            {"unit_offsets.supportPlanesCount_offset", &offsets::unit_offsets::supportPlanesCount_offset},
            {"unit_offsets.timeRepair_offset", &offsets::unit_offsets::timeRepair_offset},
            {"unit_offsets.timeToNextSmokeScreen_offset", &offsets::unit_offsets::timeToNextSmokeScreen_offset},
            {"unit_offsets.timeToRearm_offset", &offsets::unit_offsets::timeToRearm_offset},
            {"unit_offsets.torpedoDiveDepth_offset", &offsets::unit_offsets::torpedoDiveDepth_offset},
            {"unit_offsets.turret_offset", &offsets::unit_offsets::turret_offset},
            {"unit_offsets.unitArmyNo_offset", &offsets::unit_offsets::unitArmyNo_offset},
            {"unit_offsets.unitState_offset", &offsets::unit_offsets::unitState_offset},
            {"unit_offsets.visualReloadProgress_offset", &offsets::unit_offsets::visualReloadProgress_offset},
            {"unit_offsets.vulnerableByUnitUId_offset", &offsets::unit_offsets::vulnerableByUnitUId_offset},
            {"unit_offsets.wingBreakRnd_offset", &offsets::unit_offsets::wingBreakRnd_offset},
            {"unit_offsets.bodyBBMax_offset", &offsets::unit_offsets::bodyBBMax_offset},
            {"unit_offsets.bodyBBMin_offset", &offsets::unit_offsets::bodyBBMin_offset},
            {"unit_offsets.damageModelCont_offset", &offsets::unit_offsets::damageModelCont_offset},
            {"unit_offsets.invulTimer_offset", &offsets::unit_offsets::invulTimer_offset},
            {"unit_offsets.playerInfo_offset", &offsets::unit_offsets::playerInfo_offset},
            {"unit_offsets.unitIndex_offset", &offsets::unit_offsets::unitIndex_offset},
            {"unit_offsets.unitType_offset", &offsets::unit_offsets::unitType_offset},
            {"unit_offsets.ground_velocity_offset", &offsets::unit_offsets::ground_velocity_offset},
            {"unit_offsets.airmovement_offsets.velocity_offset", &offsets::unit_offsets::airmovement_offsets::velocity_offset},
        }};

        return registry;
    }

    inline auto trim(std::string value) -> std::string
    {
        const auto first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            return {};

        const auto last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1);
    }

    inline auto get_offsets_file_path() -> std::filesystem::path
    {
        char module_path[MAX_PATH]{};
        const DWORD path_len = GetModuleFileNameA(nullptr, module_path, MAX_PATH);
        if (path_len == 0)
            return std::filesystem::path("offsets.txt");

        std::filesystem::path exe_path(module_path);
        return exe_path.parent_path() / "offsets.txt";
    }

    inline auto apply_loaded_offset(const std::string& key, uintptr_t value) -> void
    {
        for (const auto& entry : runtime_offsets_registry())
        {
            if (key == entry.key)
            {
                *entry.value = value;
                return;
            }
        }
    }

    inline auto restore_default_offsets() -> void
    {
        const auto& defaults = []() -> const std::unordered_map<std::string, uintptr_t>&
        {
            static const auto snapshot = []()
            {
                std::unordered_map<std::string, uintptr_t> values;
                values.reserve(runtime_offsets_registry().size());
                for (const auto& entry : runtime_offsets_registry())
                    values.emplace(entry.key, *entry.value);
                return values;
            }();
            return snapshot;
        }();

        for (const auto& entry : runtime_offsets_registry())
        {
            const auto it = defaults.find(entry.key);
            if (it != defaults.end())
                *entry.value = it->second;
        }
    }

    inline auto save_external_offsets() -> bool
    {
        std::ofstream out(get_offsets_file_path(), std::ios::trunc);
        if (!out.is_open())
            return false;

        out << std::hex << std::showbase;
        for (const auto& entry : runtime_offsets_registry())
            out << entry.key << '=' << *entry.value << '\n';
        return true;
    }

    inline auto load_external_offsets() -> void
    {
        std::ifstream in(get_offsets_file_path());
        if (!in.is_open())
        {
            save_external_offsets();
            return;
        }

        std::string line;
        while (std::getline(in, line))
        {
            if (line.empty())
                continue;

            const auto pos = line.find('=');
            if (pos == std::string::npos || pos == 0 || pos + 1 >= line.size())
                continue;

            const std::string key = trim(line.substr(0, pos));
            const std::string value_str = trim(line.substr(pos + 1));
            if (key.empty() || value_str.empty())
                continue;

            try
            {
                const uintptr_t value = static_cast<uintptr_t>(std::stoull(value_str, nullptr, 0));
                apply_loaded_offset(key, value);
            }
            catch (...)
            {
                continue;
            }
        }
    }

    inline auto initialize_external_offsets() -> void
    {
        // Ensure defaults snapshot is captured before any external file values can overwrite runtime offsets.
        restore_default_offsets();

        if (use_external_offsets_file)
            load_external_offsets();
    }

    inline auto is_external_offsets_enabled() -> bool
    {
        return use_external_offsets_file;
    }

    inline auto set_external_offsets_enabled(bool enabled) -> void
    {
        if (use_external_offsets_file == enabled)
            return;

        use_external_offsets_file = enabled;
        if (use_external_offsets_file)
            load_external_offsets();
        else
            restore_default_offsets();
    }

    inline auto set_runtime_offset(const std::string& key, uintptr_t value) -> void
    {
        uintptr_t* target = nullptr;
        for (const auto& entry : runtime_offsets_registry())
        {
            if (key == entry.key)
            {
                target = entry.value;
                break;
            }
        }

        if (!target)
            return;

        if (*target != value)
            *target = value;
    }

    inline auto create_or_update_external_offsets_file() -> bool
    {
        return save_external_offsets();
    }

}
