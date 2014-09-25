#pragma once

#include "Application.h"
#include <glm/glm.hpp>
#include <stb_image.h>
#include <Utilities.h>
#include <GL\glew.h>

// derived application class that wraps up all globals neatly
class Tutorial3_Texturing : public Application
{
public:

	Tutorial3_Texturing();
	virtual ~Tutorial3_Texturing();

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	void LoadTexture(char * filePath);

	void CreateShaders();

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	GLuint m_texture;

	BasicVertex fuck;

	//Our vertex and index buffers
	unsigned int	  m_VAO;
	unsigned int      m_VBO;
	unsigned int	  m_IBO;

	//Where we save out shaderID
	unsigned int 	m_programID;

	unsigned int m_vertShader;
	unsigned int m_fragShader;
};