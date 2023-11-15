#pragma once
#include <defines.h>
#include <core/meminc.h>
#include <renderer/camera.h>

void ParticlelifeStart(Allocator* allocator, Camera* camera, i32 simWidth, i32 simHeight);
void ParticlelifeUpdate();
void ParticlelifeShutdown();