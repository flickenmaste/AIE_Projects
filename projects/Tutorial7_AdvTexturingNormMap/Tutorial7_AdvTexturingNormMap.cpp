#include "Tutorial7_AdvTexturingNormMap.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>
#include <stb_image.h>
#include <math.h>

using std::sin;
using std::cos;

#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

Tutorial7_AdvTexturingNormMap::Tutorial7_AdvTexturingNormMap()
{

}

Tutorial7_AdvTexturingNormMap::~Tutorial7_AdvTexturingNormMap()
{

}

bool Tutorial7_AdvTexturingNormMap::onCreate(int a_argc, char* a_argv[])
{
	// initialise the Gizmos helper class
	Gizmos::create();

	// create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse(glm::lookAt(glm::vec3(10, 10, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));

	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, DEFAULT_SCREENWIDTH / (float)DEFAULT_SCREENHEIGHT, 0.1f, 1000.0f);

	// set uniform matrices to 0
	m_MVP = glm::mat4(0);
	m_MV = glm::mat4(0);
	m_NormalMatrix = glm::mat3(0);

	// set light position
	m_lightPosition = glm::vec3(0, 1, 0);

	// set light colors
	m_ambientLightColor = glm::vec3(0.8, 0.1, 0.3);
	m_lightColor = glm::vec3(1, 1, 1);

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f, 0.25f, 0.25f, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//// texture --------------------------

	//m_decayValue = 0;

	//// load image data
	//int width = 0;
	//int height = 0;
	//int format = 0;
	//unsigned char* pixelData = stbi_load("../../assets/textures/decay.png",
	//	&width, &height, &format, 4);

	//printf("Width: %i Height: %i Format: %i\n", width, height, format);

	//// create OpenGL texture handle
	//glGenTextures(1, &m_decayTexture);
	//glBindTexture(GL_TEXTURE_2D, m_decayTexture);

	//// set pixel data for texture
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);

	//// set filtering
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//// clear bound texture state so we don't accidentally change it
	//glBindTexture(GL_TEXTURE_2D, 0);

	//delete[] pixelData;

	//// -----------------------------------

	// load the shader
	const char* aszInputs[] = { "Position", "Normal", "Tangent", "BiNormal", "TexCoord1" };
	const char* aszOutputs[] = { "outColor" };

	// load shader internally calls glCreateShader...
	GLuint vshader = Utility::loadShader("./shaders/vertShader.vert", GL_VERTEX_SHADER);
	GLuint fshader = Utility::loadShader("./shaders/fragShader.frag", GL_FRAGMENT_SHADER);

	m_shader = Utility::createProgram(vshader, 0, 0, 0, fshader, 5, aszInputs, 1, aszOutputs);

	// free our shader once we built our program
	glDeleteShader(vshader);
	glDeleteShader(fshader);

	m_fbx = new FBXFile();
	m_fbx->load("./Models/soulspear/soulspear.fbx", FBXFile::UNITS_CENTIMETER);
	m_fbx->initialiseOpenGLTextures();
	InitFBXSceneResource(m_fbx);

	return true;
}

void Tutorial7_AdvTexturingNormMap::onUpdate(float a_deltaTime)
{
	// update our camera matrix using the keyboard/mouse
	Utility::freeMovement(m_cameraMatrix, a_deltaTime, 10);

	// clear all gizmos from last frame
	Gizmos::clear();

	// add an identity matrix gizmo
	Gizmos::addTransform(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));

	// add a 20x20 grid on the XZ-plane
	for (int i = 0; i < 21; ++i)
	{
		Gizmos::addLine(glm::vec3(-10 + i, 0, 10), glm::vec3(-10 + i, 0, -10),
			i == 10 ? glm::vec4(1, 1, 1, 1) : glm::vec4(0, 0, 0, 1));

		Gizmos::addLine(glm::vec3(10, 0, -10 + i), glm::vec3(-10, 0, -10 + i),
			i == 10 ? glm::vec4(1, 1, 1, 1) : glm::vec4(0, 0, 0, 1));
	}

	UpdateFBXSceneResource(m_fbx);

	m_lightPosition.z = sin(glfwGetTime()) * 2.5f;
	m_lightPosition.x = cos(glfwGetTime()) * 2.5f;

	// quit our application when escape is pressed
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();
}

void Tutorial7_AdvTexturingNormMap::onDraw()
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);

	// draw the gizmos from this frame
	Gizmos::draw(m_projectionMatrix, viewMatrix);

	// get window dimensions for 2D orthographic projection
	int width = 0, height = 0;
	glfwGetWindowSize(m_window, &width, &height);
	Gizmos::draw2D(glm::ortho<float>(0, width, 0, height, -1.0f, 1.0f));

	RenderFBXSceneResource(m_fbx, viewMatrix, m_projectionMatrix);
}

void Tutorial7_AdvTexturingNormMap::onDestroy()
{
	// clean up anything we created
	Gizmos::destroy();

	glDeleteShader(m_shader);

	DestroyFBXSceneResource(m_fbx);
	m_fbx->unload();
	delete m_fbx;
	m_fbx = NULL;

}

// main that controls the creation/destruction of an application
int main(int argc, char* argv[])
{
	// explicitly control the creation of our application
	Application* app = new Tutorial7_AdvTexturingNormMap();

	if (app->create("AIE - Tutorial7_AdvTexturingNormMap", DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT, argc, argv) == true)
		app->run();

	// explicitly control the destruction of our application
	delete app;

	return 0;
}

void Tutorial7_AdvTexturingNormMap::InitFBXSceneResource(FBXFile *a_pScene)
{
	// how many meshes and materials are stored within the fbx file
	unsigned int meshCount = a_pScene->getMeshCount();
	unsigned int matCount = a_pScene->getMaterialCount();

	// loop through each mesh
	for (int i = 0; i < meshCount; ++i)
	{
		// get the current mesh
		FBXMeshNode *pMesh = a_pScene->getMeshByIndex(i);

		// generate our OGL_FBXRenderData for storing the meshes VBO, IBO, and VAO
		// and assign it to the meshes m_userData pointer so it can be retrieved
		// again within the render function
		OGL_FBX_RenderData *ro = new OGL_FBX_RenderData();
		pMesh->m_userData = ro;

		// OPENGL: generate the VBO, IBO, and VAO
		glGenBuffers(1, &ro->VBO);
		glGenBuffers(1, &ro->IBO);
		glGenVertexArrays(1, &ro->VAO);

		// OpenGL: Bind VAO, then bind the VBO and IBO to the VAO
		glBindVertexArray(ro->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, ro->VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ro->IBO);

		// send the vertex data to the VBO
		glBufferData(GL_ARRAY_BUFFER, pMesh->m_vertices.size() * sizeof(FBXVertex), pMesh->m_vertices.data(), GL_STATIC_DRAW);

		// send the index data to the IBO
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh->m_indices.size() * sizeof(unsigned int), pMesh->m_indices.data(), GL_STATIC_DRAW);

		// enable the attribute locations that will be used on our shaders
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);

		// tell our shaders where the information within our buffers lie
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::PositionOffset);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::NormalOffset);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::TangentOffset);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::BiNormalOffset);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::TexCoord1Offset);

		glBindVertexArray(0);
	}
}

void Tutorial7_AdvTexturingNormMap::DestroyFBXSceneResource(FBXFile *a_pScene)
{
	// how many meshes and materials are stored within the fbx file
	unsigned int meshCount = a_pScene->getMeshCount();
	unsigned int matCount = a_pScene->getMaterialCount();

	// remove meshes
	for (unsigned int i = 0; i < meshCount; i++)
	{
		// Get the current mesh and retrieve the render data we assigned to m_userData
		FBXMeshNode* pMesh = a_pScene->getMeshByIndex(i);
		OGL_FBX_RenderData *ro = (OGL_FBX_RenderData *)pMesh->m_userData;

		// delete the buffers and free memory from the graphics card
		glDeleteBuffers(1, &ro->VBO);
		glDeleteBuffers(1, &ro->IBO);
		glDeleteVertexArrays(1, &ro->VAO);

		// this is memory we creaded earlier in the InitBVXSceneResources function
		// make sure to destroy it
		delete ro;
	}

	// loop through each of the materials
	for (int i = 0; i < matCount; i++)
	{
		// get the current material 
		FBXMaterial *pMaterial = a_pScene->getMaterialByIndex(i);
		for (int j = 0; j < FBXMaterial::TextureTypes_Count; j++)
		{
			// delete the texture if it was loaded
			//if (pMaterial->textureIDs[j] != 0)
			//glDeleteTextures(1, &pMaterial->textureIDs[j]);
		}
	}

}

void Tutorial7_AdvTexturingNormMap::UpdateFBXSceneResource(FBXFile *a_pScene)
{
	a_pScene->getRoot()->updateGlobalTransform();
}

void Tutorial7_AdvTexturingNormMap::RenderFBXSceneResource(FBXFile *a_pScene, glm::mat4 a_view, glm::mat4 a_projection)
{
	// enable transparent blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// activate a shader
	glUseProgram(m_shader);

	// get the location of uniforms on the shader
	// vert ---
	GLint uMVP = glGetUniformLocation(m_shader, "MVP");
	GLint uMV = glGetUniformLocation(m_shader, "MV");
	GLint uNormalMatrix = glGetUniformLocation(m_shader, "NormalMatrix");

	GLint uLightPosition = glGetUniformLocation(m_shader, "LightPosition");

	// frag ---
	GLint uDiffuseTexture = glGetUniformLocation(m_shader, "DiffuseTexture");
	GLint uNormalTexture = glGetUniformLocation(m_shader, "NormalTexture");
	GLint uSpecularTexture = glGetUniformLocation(m_shader, "SpecularTexture");

	GLint uAmbientLightColor = glGetUniformLocation(m_shader, "AmbientLightColor");
	GLint uLightColor = glGetUniformLocation(m_shader, "LightColor");

	//GLint uToggle = glGetUniformLocation(m_shader, "toggle");

	// for each mesh in the model
	for (int i = 0; i < a_pScene->getMeshCount(); ++i)
	{
		// get the current mesh
		FBXMeshNode *mesh = a_pScene->getMeshByIndex(i);

		// get the render data attached to the m_userData pointer for this mesh
		OGL_FBX_RenderData *ro = (OGL_FBX_RenderData *)mesh->m_userData;

		// calculate matrices ** right spot for this?? **
		m_MVP = a_projection * a_view * (mesh->m_globalTransform);
		m_MV = a_view * (mesh->m_globalTransform);
		m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(m_MV)));

		// send matrices to shader
		glUniformMatrix4fv(uMVP, 1, false, glm::value_ptr(m_MVP));
		glUniformMatrix4fv(uMV, 1, false, glm::value_ptr(m_MV));

		glUniformMatrix3fv(uNormalMatrix, 1, false, glm::value_ptr(m_NormalMatrix));

		// send light position to shader
		glUniform3fv(uLightPosition, 1, glm::value_ptr(m_lightPosition));

		// Bind the texture to one of the ActiveTextures
		// if your shader supported multiple textures, you would bind each texture to a new Active Texture ID here
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mesh->m_material->textures[FBXMaterial::DiffuseTexture]->handle);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mesh->m_material->textures[FBXMaterial::NormalTexture]->handle);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, mesh->m_material->textures[FBXMaterial::SpecularTexture]->handle);

		// TELL THE SHADER WHICH TEXTURE TO USE
		glUniform1i(uDiffuseTexture, 1);
		glUniform1i(uNormalTexture, 2);
		glUniform1i(uSpecularTexture, 3);

		// send light colors
		glUniform3fv(uAmbientLightColor, 1, glm::value_ptr(m_ambientLightColor));
		glUniform3fv(uLightColor, 1, glm::value_ptr(m_lightColor));

		// bind vertex array object
		// remember in the initialize function, we bound the VBO and IBO to the VAO
		// so when we bind the VAO, openGL knows what vertices, indices and vertex attributes 
		// to send to the shader
		glBindVertexArray(ro->VAO);
		glDrawElements(GL_TRIANGLES, mesh->m_indices.size(), GL_UNSIGNED_INT, 0);
	}

	// reset back to the default active texture
	glActiveTexture(GL_TEXTURE0);

	// finished rendering meshes, disable shader
	glUseProgram(0);
}