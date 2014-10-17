#pragma once

#include "Application.h"
#include <glm/glm.hpp>
#include <stb_image.h>
#include <Utilities.h>
#include <FBXFile.h>
#include <GL\glew.h>

// derived application class that wraps up all globals neatly
class Tutorial8_Tessellation : public Application
{
public:

	Tutorial8_Tessellation();
	virtual ~Tutorial8_Tessellation();

	void InitFBXSceneResource(FBXFile *a_pScene);
	void UpdateFBXSceneResource(FBXFile *a_pScene);
	void RenderFBXSceneResource(FBXFile *a_pScene, glm::mat4 a_view, glm::mat4 a_projection);
	void DestroyFBXSceneResource(FBXFile *a_pScene);

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	void LoadTexture(char * filePath);

	void CreateShaders();

	void CreateTessellation();

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;
	glm::mat4	m_projectionView;

	unsigned int m_shader;

	struct OGL_FBXRenderData
	{
		unsigned int VBO; // vertex buffer object
		unsigned int IBO; // index buffer object
		unsigned int VAO; // vertex array object
	};

	FBXFile *m_fbx;

	GLuint m_texture;
	GLuint m_texture2;

	float m_decayValue;

	BasicVertex fuck;

	//Our vertex and index buffers
	unsigned int	  m_VAO;
	unsigned int      m_VBO;
	unsigned int	  m_IBO;

	//Where we save out shaderID
	unsigned int 	m_programID;

	unsigned int m_vertShader;
	unsigned int m_fragShader;

	GLuint m_displacement;
};