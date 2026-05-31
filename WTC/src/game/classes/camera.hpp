#pragma once

class c_camera {
public:
	//auto init( uintptr_t cGame ) -> bool {
	//	this->base_address = TargetProcess->Read< uintptr_t >( cGame + offsets::cgame_offsets::camera_offset );
	//	return this->base_address != 0;
	//}

	auto init(uintptr_t cGame) -> bool
	{
		uintptr_t addr = cGame + offsets::cgame_offsets::camera_offset;
		this->base_address = TargetProcess->Read<uintptr_t>(addr);
		return (this->base_address != 0);
	}

public:

    auto getPosition() -> vec3_t
    {
        // CCamera::camera_position = 0x58
        return TargetProcess->Read<vec3_t>(
            this->base_address
            + offsets::cgame_offsets::camera_offsets::camera_position_offset
        );
    }

    auto getCameraMatrix() -> ViewMatrix_t
    {
        VMMDLL_SCATTER_HANDLE hScatter = TargetProcess->CreateScatterHandle();
        if (!hScatter)
            return ViewMatrix_t();

        ViewMatrix_t matrix;
        if (!TargetProcess->AddScatterReadRequest(
            hScatter,
            uintptr_t(this->base_address + offsets::cgame_offsets::camera_offsets::camera_matrix_offset),
            &matrix,
            sizeof(ViewMatrix_t)) ||
            !TargetProcess->ExecuteReadScatter(hScatter, 0, true))
        {
            TargetProcess->CloseScatterHandle(hScatter);
            return ViewMatrix_t();
        }

        return matrix;
    }

private:
	uintptr_t base_address;
};
