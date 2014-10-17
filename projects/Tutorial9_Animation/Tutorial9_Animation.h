#pragma once

#include "Application.h"
#include <glm/glm.hpp>
#include <stb_image.h>
#include <Utilities.h>
#include <FBXFile.h>
#include <GL\glew.h>

// derived application class that wraps up all globals neatly
class Tutorial9_Animation : public Application
{
public:

	Tutorial9_Animation();
	virtual ~Tutorial9_Animation();

	void InitFBXSceneResource(FBXFile *a_pScene);
	void UpdateFBXSceneResource(FBXFile *a_pScene);
	void RenderFBXSceneResource(FBXFile *a_pScene, glm::mat4 a_view, glm::mat4 a_projection);
	void DestroyFBXSceneResource(FBXFile *a_pScene);

protected:

	virtual bool onCreate(int a_argc, char* a_argv[]);
	virtual void onUpdate(float a_deltaTime);
	virtual void onDraw();
	virtual void onDestroy();

	glm::mat4	m_cameraMatrix;
	glm::mat4	m_projectionMatrix;
	glm::mat4	m_projectionView;

	// Custom code ***

	unsigned int m_shader;

	struct OGL_FBXRenderData
	{
		unsigned int VBO; // vertex buffer object
		unsigned int IBO; // index buffer object
		unsigned int VAO; // vertex array object
	};

	FBXFile *m_fbx;
};