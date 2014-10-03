#pragma once

#include "Application.h"
#include <glm/glm.hpp>
#include <Utilities.h>
#include <GL\glew.h>
#include <FBXFile.h>

#include "DirectionalLight.h";

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

	//void LoadTexture(char * filePath);

	void CreateShaders();

	void createOpenGLBuffers(FBXFile* a_fbx);

	void cleanupOpenGLBuffers(FBXFile* a_fbx);

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	GLuint m_texture;

	BasicVertex fuck;

	FBXFile *m_fbx;

	//Our vertex and index buffers
	unsigned int	  m_VAO;
	unsigned int      m_VBO;
	unsigned int	  m_IBO;

	//Where we save out shaderID
	unsigned int 	m_programID;

	unsigned int m_vertShader;
	unsigned int m_fragShader;

	glm::vec3 lightAmbient;
	DirectionalLight lightDirection;
	glm::vec3 lightColor;
	glm::vec3 lightSpecular;
};