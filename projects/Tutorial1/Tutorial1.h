#pragma once

#include "Application.h"
#include <glm/glm.hpp>

// derived application class that wraps up all globals neatly
class Tutorial1 : public Application
{
public:

	Tutorial1();
	virtual ~Tutorial1();

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();
	virtual void CheckForInput(GLFWwindow*, float deltaTime);

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;
	
	glm::mat4	m_customMatrix;
	glm::vec3	rotPosition;
};