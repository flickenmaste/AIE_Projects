#pragma once

#include "Application.h"
#include <glm/glm.hpp>

// derived application class that wraps up all globals neatly
class Tutorial2 : public Application
{
public:

	Tutorial2();
	virtual ~Tutorial2();
	void GenerateGrid(unsigned int rows, unsigned int cols);
	void CreateShaders();

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;

	// Custom stuff ***

	// AKA Vertex struct
	struct Vertex4p4c
	{
		glm::vec4 position;
		glm::vec4 colour;
	};

	//Our vertex and index buffers
	unsigned int	  m_VAO;
	unsigned int      m_VBO;
	unsigned int	  m_IBO;

	//Where we save out shaderID
	unsigned int 	m_programID;
};