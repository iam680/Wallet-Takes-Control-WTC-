#pragma once
#include <corecrt_math_defines.h>

namespace aimbot
{
    struct AimCacheEntry
    {
        vec3_t aim_point{};
        vec3_t raw_aim_point{};
        std::chrono::steady_clock::time_point next_update{};
        bool initialized = false;
    };

    struct BallisticsData
    {
        float mass = 0.0f;
        float caliber = 0.0f;
        float velocity = 0.0f;
        float length = 0.0f;

        float max_dist = 0.0f;
    };

    inline BallisticsData ballisticsData;

    inline auto GetBallisticsInfo() -> void
    {
        ballisticsData.mass = sdk::cGame->ballistics->getMass();
        ballisticsData.caliber = sdk::cGame->ballistics->getCaliber();
        ballisticsData.velocity = sdk::cGame->ballistics->getVelocity();
        ballisticsData.length = sdk::cGame->ballistics->getLength();
        ballisticsData.max_dist = sdk::cGame->ballistics->getMaxDistance();
    }

    inline auto BeginFrame() -> void
    {
        static std::chrono::steady_clock::time_point next_update{};
        const auto now = std::chrono::steady_clock::now();
        if (now < next_update)
            return;

        GetBallisticsInfo();
        next_update = now + std::chrono::milliseconds(50);
    }

    class BallisticsPrediction {
    private:
        static constexpr float GRAVITY = -9.81f;
        static constexpr float TIME_STEP = 1.0f / 96.0f; // High precision timestep
        static constexpr float MAX_SIMULATION_TIME = 10.0f;
        static constexpr float CONVERGENCE_THRESHOLD = 0.01f;
        static constexpr int MAX_ITERATIONS = 50;

        // Enhanced drag calculation with altitude compensation
        float GetDragConstant(float altitude) const {
            const float max_alt = 18300.0f;
            const float alt_mult = 1.225f;
            const auto clamped_alt = std::fmin( altitude, max_alt );

            // Atmospheric density polynomial coefficients
            const float c1 = 2.2871901e-19f;
            const float c2 = 5.8355603e-14f;
            const float c3 = 0.00000000353118f;
            const float c4 = 0.000095938703f;

            return alt_mult *
                ((max_alt / std::fmax(altitude, max_alt)) *
                    ((((((c1 * clamped_alt) - c2) * clamped_alt) + c3) *
                        clamped_alt) -
                        c4) *
                    clamped_alt +
                    1.0f);
        }

        float GetBallisticCoefficient(BallisticsData data, float dragConstant) const {
            //const float cross_sectional_area = M_PI * std::pow(data.caliber * 0.5f, 2.0f);
            //return (dragConstant * cross_sectional_area * data.length) / data.mass;

            return -1.0f * ( dragConstant * static_cast< float >( M_PI ) * 0.5f * std::pow( data.caliber * 0.5f, 2.0f ) * data.length ) / data.mass;

        }

        // Apply drag and gravity to projectile
        void ApplyPhysics(vec3_t& position, vec3_t& velocity, float ballisticCoeff, float deltaTime) const {
            // Calculate drag
            float speed = velocity.length();
            if (speed > 0.01f) {
                float dragForce = ballisticCoeff * speed * speed;
                vec3_t normalizedVel = velocity.normalized();
                vec3_t dragAcceleration = normalizedVel * (-dragForce);
                velocity = velocity + dragAcceleration * deltaTime;
            }

            // Apply gravity
            velocity.y += GRAVITY * deltaTime;

            // Update position
            position = position + velocity * deltaTime;
        }

        // Simulate projectile trajectory
        bool SimulateTrajectory(vec3_t startPos, vec3_t initialVelocity, vec3_t targetPos,
            float ballisticCoeff, float maxTime, vec3_t& hitPoint, float& hitTime) const {
            vec3_t pos = startPos;
            vec3_t vel = initialVelocity;
            float time = 0;

            vec3_t lastPos = pos;

            while (time < maxTime) {
                lastPos = pos;
                ApplyPhysics(pos, vel, ballisticCoeff, TIME_STEP);
                time += TIME_STEP;

                // Check if we've passed the target horizontally
                vec3_t toTarget = targetPos - startPos;
                vec3_t toCurrentPos = pos - startPos;

                float targetDist2D = std::sqrt(toTarget.x * toTarget.x + toTarget.z * toTarget.z);
                float currentDist2D = std::sqrt(toCurrentPos.x * toCurrentPos.x + toCurrentPos.z * toCurrentPos.z);

                if (currentDist2D >= targetDist2D) {
                    // Interpolate hit point
                    float alpha = (targetDist2D - std::sqrt((lastPos - startPos).x * (lastPos - startPos).x +
                        (lastPos - startPos).z * (lastPos - startPos).z)) /
                        (currentDist2D - std::sqrt((lastPos - startPos).x * (lastPos - startPos).x +
                            (lastPos - startPos).z * (lastPos - startPos).z));

                    hitPoint = lastPos + (pos - lastPos) * alpha;
                    hitTime = time - TIME_STEP + TIME_STEP * alpha;
                    return true;
                }

                // Stop if projectile hits ground (assuming ground at y=0 or below target)
                if (pos.y < min(startPos.y - 1000.0f, targetPos.y - 100.0f)) {
                    break;
                }
            }

            hitPoint = pos;
            hitTime = time;
            return false;
        }

    public:
        // Main prediction function with iterative target position estimation
        vec3_t PredictInterceptPoint(vec3_t shooterPos, vec3_t targetPos, vec3_t targetVelocity,
            const BallisticsData& ballistics) const {

            if (ballistics.velocity <= 0) return targetPos;

            float altitude = max(shooterPos.y, targetPos.y);
            float dragConstant = GetDragConstant( altitude );
            float ballisticCoeff = GetBallisticCoefficient(ballistics, dragConstant);

            vec3_t predictedTargetPos = targetPos;
            vec3_t lastPrediction = targetPos;

            // Iterative prediction
            for (int iteration = 0; iteration < MAX_ITERATIONS; iteration++) {
                vec3_t direction = (predictedTargetPos - shooterPos).normalized();
                vec3_t initialVelocity = direction * ballistics.velocity;

                vec3_t hitPoint;
                float hitTime;

                if (SimulateTrajectory(shooterPos, initialVelocity, predictedTargetPos,
                    ballisticCoeff, MAX_SIMULATION_TIME, hitPoint, hitTime)) {

                    // Update target position prediction based on calculated flight time
                    vec3_t newPrediction = targetPos + targetVelocity * hitTime;

                    // Check convergence
                    float predictionError = (newPrediction - predictedTargetPos).length();
                    if (predictionError < CONVERGENCE_THRESHOLD) {
                        return newPrediction;
                    }

                    // idk maybe this is trash
                    float drop = 0.5f * std::abs(GRAVITY) * hitTime * hitTime;
                    newPrediction.y += drop;

                    predictedTargetPos = newPrediction;
                }
                else {
                    // If simulation failed, use simpler prediction
                    float estimatedTime = shooterPos.dist_to(predictedTargetPos) / ballistics.velocity;
                    predictedTargetPos = targetPos + targetVelocity * estimatedTime;
                    break;
                }
            }

            return predictedTargetPos;
        }

        // Calculate optimal firing angle for long-range shots
        float CalculateOptimalAngle(vec3_t shooterPos, vec3_t targetPos, float muzzleVelocity,
            const BallisticsData& ballistics) const {
            vec3_t displacement = targetPos - shooterPos;
            float horizontalDist = std::sqrt(displacement.x * displacement.x + displacement.z * displacement.z);
            float verticalDist = displacement.y;

            float altitude = max(shooterPos.y, targetPos.y);
            float dragConstant = GetDragConstant(altitude);
            float ballisticCoeff = GetBallisticCoefficient(ballistics, dragConstant);

            // Binary search for optimal angle
            float lowAngle = -M_PI / 6.0f;  // -30 degrees
            float highAngle = M_PI / 3.0f;   // 60 degrees
            float bestAngle = 0;
            float minError = FLT_MAX;

            for (int i = 0; i < 30; i++) {
                float testAngle = (lowAngle + highAngle) * 0.5f;

                vec3_t direction = vec3_t(
                    horizontalDist > 0 ? displacement.x / horizontalDist : 1.0f,
                    std::sin(testAngle),
                    horizontalDist > 0 ? displacement.z / horizontalDist : 0.0f
                ).normalized();

                vec3_t initialVel = direction * muzzleVelocity;
                vec3_t hitPoint;
                float hitTime;

                if (SimulateTrajectory(shooterPos, initialVel, targetPos, ballisticCoeff,
                    MAX_SIMULATION_TIME, hitPoint, hitTime)) {
                    float error = (hitPoint - targetPos).length();
                    if (error < minError) {
                        minError = error;
                        bestAngle = testAngle;
                    }

                    if (hitPoint.y > targetPos.y) {
                        highAngle = testAngle;
                    }
                    else {
                        lowAngle = testAngle;
                    }
                }
                else {
                    highAngle = testAngle;
                }
            }

            return bestAngle;
        }

        // Get bullet drop compensation
        vec3_t GetAimPoint(vec3_t shooterPos, vec3_t targetPos, const BallisticsData& ballistics) const {
            vec3_t direction = (targetPos - shooterPos);
            float distance = direction.length();
            direction.normalized();

            float altitude = max(shooterPos.y, targetPos.y);
            float dragConstant = GetDragConstant(altitude);
            float ballisticCoeff = GetBallisticCoefficient(ballistics, dragConstant);

            // Estimate flight time
            float flightTime = distance / ballistics.velocity;

            // Calculate bullet drop
            float drop = 0.5f * std::abs(GRAVITY) * flightTime * flightTime;

            // Apply drag effect (simplified)
            float dragEffect = ballisticCoeff * distance * 0.1f;

            // Compensate aim point
            vec3_t aimPoint = targetPos;
            aimPoint.y += drop + dragEffect;

            return aimPoint;
        }
    };

    inline auto run( c_unit& unit, vec3_t unit_position, vec3_t local_position, ViewMatrix_t camera_matrix ) -> void {
        static BallisticsPrediction pred;
        static std::unordered_map<uintptr_t, AimCacheEntry> aim_cache;
        constexpr float smooth_alpha = 0.30f;

        const auto now = std::chrono::steady_clock::now();
        const uintptr_t unit_base = unit.get_base();
        auto& cache = aim_cache[unit_base];

        vec3_t targetVelocity = unit.get_movement_ground( ).velocity( );
        const float targetSpeed = targetVelocity.length( );

        if (now >= cache.next_update)
        {
            float horizontalDist = std::sqrt(
                ( unit_position.x - local_position.x ) * ( unit_position.x - local_position.x ) +
                ( unit_position.z - local_position.z ) * ( unit_position.z - local_position.z )
            );

            float distanceFactor = horizontalDist / 150.0f;
            float extraOffset = distanceFactor * 0.143f;

            vec3_t adjusted_position = unit_position;
            adjusted_position.y += 1.0f + extraOffset;

            if ( targetSpeed > 0.1f )
                cache.raw_aim_point = pred.PredictInterceptPoint( local_position, adjusted_position, targetVelocity, ballisticsData );
            else
                cache.raw_aim_point = pred.GetAimPoint( local_position, adjusted_position, ballisticsData );

            if ( !cache.initialized )
            {
                cache.aim_point = cache.raw_aim_point;
                cache.initialized = true;
            }

            cache.next_update = now + ( targetSpeed > 0.1f ? std::chrono::milliseconds(60) : std::chrono::milliseconds(120) );
        }

        if ( cache.initialized )
            cache.aim_point = cache.aim_point + ( cache.raw_aim_point - cache.aim_point ) * smooth_alpha;

        const vec3_t& aimPoint = cache.aim_point;
            
        vec2_t screen;
        if ( !g_render->world_to_screen( aimPoint, screen, camera_matrix ) )
            return;

        g_render->circle_filled( screen.x, screen.y, 3.0f, IM_COL32( 255, 0, 0, 255 ), 20 );

        vec2_t unitScreen;
        if ( g_render->world_to_screen( unit_position, unitScreen, camera_matrix ) )
            g_render->line( unitScreen.x, unitScreen.y, screen.x, screen.y, IM_COL32( 255, 0, 0, 24 ), 1.0f );
        
    }
}
