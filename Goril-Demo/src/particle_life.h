#pragma once
#include <defines.h>
#include <core/meminc.h>
#include <renderer/camera.h>

void ParticlelifeStart(Allocator* allocator, Camera* camera);
void ParticlelifeUpdate();
void ParticlelifeShutdown();