#pragma once

#include "Application.h"
#include <glm/glm.hpp>
#include "GPUParticleSystem.h"

// derived application class that wraps up all globals neatly
class Tutorial6_GPUParticles : public Application
{
public:

	Tutorial6_GPUParticles();
	virtual ~Tutorial6_GPUParticles();

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	GPUParticleEmitter *m_emitter;
};