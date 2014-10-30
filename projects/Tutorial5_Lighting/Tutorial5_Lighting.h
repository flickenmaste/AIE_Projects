#pragma once

#include "Application.h"
#include <glm/glm.hpp>
#include "FBXFile.h"
#include "DirectionalLight.h"
#include "PointLight.h"

// derived application class that wraps up all globals neatly
class Tutorial5_Lighting : public Application
{
public:

	Tutorial5_Lighting();
	virtual ~Tutorial5_Lighting();

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	void createOpenGLBuffers(FBXFile* a_fbx);
	void cleanupOpenGLBuffers(FBXFile* a_fbx);

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	unsigned int m_vertShader, m_fragShader, m_programID;

	FBXFile *m_fbx;

	DirectionalLight m_dLight;
	glm::vec3 m_aLight;
	PointLight m_pLight;
};