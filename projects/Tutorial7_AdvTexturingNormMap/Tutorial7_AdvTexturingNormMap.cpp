#include "Tutorial7_AdvTexturingNormMap.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

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

	// get window dimensions to calculate aspect ratio
	int width = 0, height = 0;
	glfwGetWindowSize(m_window, &width, &height);

	// create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, width / (float)height, 0.1f, 1000.0f);

	// set the clear colour and enable depth testing and backface culling
	glClearColor(0.25f, 0.25f, 0.25f, 1);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	// Custom code ***

	// load the shader
	const char* aszInputs[] = { "Position", "Normal", "Tangent", "BiNormal", "TexCoord1" };
	const char* aszOutputs[] = { "outColour" };

	// load shader internally calls glCreateShader...
	GLuint vshader = Utility::loadShader("./shaders/vertShader.vert", GL_VERTEX_SHADER);
	GLuint pshader = Utility::loadShader("./shaders/fragShader.frag", GL_FRAGMENT_SHADER);

	m_shader = Utility::createProgram(vshader, 0, 0, 0, pshader, 5, aszInputs, 1, aszOutputs);

	// free our shader once we built our program
	glDeleteShader(vshader);
	glDeleteShader(pshader);

	m_fbx = new FBXFile();
	m_fbx->load("./Models/soulspear/soulspear.fbx", FBXFile::UNITS_CENTIMETER);
	m_fbx->initialiseOpenGLTextures();
	InitFBXSceneResource(m_fbx);

	//LoadTexture("./textures/decay.png");

	lightAmbient = glm::vec3(1.0f, 0.2f, 0.7f);
	lightDirection.Direction = glm::vec3(1, 1, 1);
	lightDirection.Color = glm::vec3(0.5, 1, 1);
	lightSpecular = glm::vec3(0.5f, 0.5f, 0.5f);
	m_lightPos = glm::vec3(1, 1, 1);
	m_lightColor = glm::vec3(0.5, 1, 1);


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

	// quit our application when escape is pressed
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();

	UpdateFBXSceneResource(m_fbx);
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
	// how manu meshes and materials are stored within the fbx file
	unsigned int meshCount = a_pScene->getMeshCount();
	unsigned int matCount = a_pScene->getMaterialCount();

	// loop through each mesh
	for (int i = 0; i<meshCount; ++i)
	{
		// get the current mesh
		FBXMeshNode *pMesh = a_pScene->getMeshByIndex(i);

		// genorate our OGL_FBXRenderData for storing the meshes VBO, IBO and VAO
		// and assign it to the meshes m_userData pointer so that we can retrive 
		// it again within the render function
		OGL_FBXRenderData *ro = new OGL_FBXRenderData();
		pMesh->m_userData = ro;

		// OPENGL: genorate the VBO, IBO and VAO
		glGenBuffers(1, &ro->VBO);
		glGenBuffers(1, &ro->IBO);
		glGenVertexArrays(1, &ro->VAO);

		// OPENGL: Bind  VAO, and then bind the VBO and IBO to the VAO
		glBindVertexArray(ro->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, ro->VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ro->IBO);

		// Send the vertex data to the VBO
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
		// eg: attrubute 0 is expected to be the verticy's position. it should be 4 floats, representing xyzw
		// eg: attrubute 1 is expected to be the verticy's color. it should be 4 floats, representing rgba
		// eg: attrubute 2 is expected to be the verticy's texture coordinate. it should be 2 floats, representing U and V
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char *)FBXVertex::PositionOffset);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char *)FBXVertex::NormalOffset);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char *)FBXVertex::TangentOffset);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char *)FBXVertex::BiNormalOffset);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char *)FBXVertex::TexCoord1Offset);

		// finally, where done describing our mesh to the shader
		// we can describe the next mesh
		glBindVertexArray(0);
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
	GLint uDiffuseTexture = glGetUniformLocation(m_shader, "DiffuseTexture");
	GLint uNormalTexture = glGetUniformLocation(m_shader, "NormalTexture");
	GLint uSpecularTexture = glGetUniformLocation(m_shader, "SpecularTexture");

	GLint uMVP = glGetUniformLocation(m_shader, "MVP");
	GLint uMV = glGetUniformLocation(m_shader, "MV");
	GLint uNormalMatrix = glGetUniformLocation(m_shader, "NormalMatrix");

	GLint uLightPos = glGetUniformLocation(m_shader, "LightPosition");
	GLint uAmbientLight = glGetUniformLocation(m_shader, "AmbientLightColor");
	GLint uLightColor = glGetUniformLocation(m_shader, "LightColor");

	// for each mesh in the model...
	for (int i = 0; i<a_pScene->getMeshCount(); ++i)
	{
		// get the current mesh
		FBXMeshNode *mesh = a_pScene->getMeshByIndex(i);

		// get the render data attached to the m_userData pointer for this mesh
		OGL_FBXRenderData *ro = (OGL_FBXRenderData *)mesh->m_userData;

		// Bind the texture to one of the ActiveTextures
		// if your shader supported multiple textures, you would bind each texture to a new Active Texture ID here
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mesh->m_material->textures[FBXMaterial::DiffuseTexture]->handle);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mesh->m_material->textures[FBXMaterial::NormalTexture]->handle);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, mesh->m_material->textures[FBXMaterial::SpecularTexture]->handle);

		glActiveTexture(GL_TEXTURE0);

		// tell the shader which texture to use
		glUniform1i(uDiffuseTexture, 1);
		glUniform1i(uNormalTexture, 2);
		glUniform1i(uSpecularTexture, 3);

		m_MVP = a_projection * a_view * mesh->m_globalTransform;
		m_MV = a_view * mesh->m_globalTransform;
		m_NormalMatrix = glm::transpose(glm::inverse(glm::mat3(m_MV)));

		// send the Model, View and Projection Matrices to the shader
		glUniformMatrix4fv(uMVP, 1, false, glm::value_ptr(m_MVP));
		glUniformMatrix4fv(uMV, 1, false, glm::value_ptr(m_MV));
		glUniformMatrix3fv(uNormalMatrix, 1, false, glm::value_ptr(m_NormalMatrix));

		glUniform3fv(uLightPos, 1, glm::value_ptr(m_lightPos));
		glUniform3f(uAmbientLight, lightAmbient.x, lightAmbient.y, lightAmbient.z);
		glUniform3f(uLightColor, m_lightColor.x, m_lightColor.y, m_lightColor.z);

		// bind our vertex array object
		// remember in the initialise function, we bound the VAO and IBO to the VAO
		// so when we bind the VAO, openGL knows what what vertices,
		// indices and vertex attributes to send to the shader
		glBindVertexArray(ro->VAO);
		glDrawElements(GL_TRIANGLES, mesh->m_indices.size(), GL_UNSIGNED_INT, 0);

	}

	// reset back to the default active texture
	glActiveTexture(GL_TEXTURE0);

	// finally, we have finished rendering the meshes
	// disable this shader
	glUseProgram(0);
}

void Tutorial7_AdvTexturingNormMap::DestroyFBXSceneResource(FBXFile *a_pScene)
{
	// how manu meshes and materials are stored within the fbx file
	unsigned int meshCount = a_pScene->getMeshCount();
	unsigned int matCount = a_pScene->getMaterialCount();

	// remove meshes
	for (unsigned int i = 0; i<meshCount; i++)
	{
		// Get the current mesh and retrive the render data we assigned to m_userData
		FBXMeshNode* pMesh = a_pScene->getMeshByIndex(i);
		OGL_FBXRenderData *ro = (OGL_FBXRenderData *)pMesh->m_userData;

		// delete the buffers and free memory from the graphics card
		glDeleteBuffers(1, &ro->VBO);
		glDeleteBuffers(1, &ro->IBO);
		glDeleteVertexArrays(1, &ro->VAO);

		// this is memory we created earlier in the InitFBXSceneResources function
		// make sure to destroy it
		delete ro;

	}

	// loop through each of the materials
	for (int i = 0; i<matCount; i++)
	{
		// get the current material
		FBXMaterial *pMaterial = a_pScene->getMaterialByIndex(i);
		for (int j = 0; j<FBXMaterial::TextureTypes_Count; j++)
		{
			// delete the texture if it was loaded
			//if (pMaterial->textureIDs[j] != 0)
			//glDeleteTextures(1, &pMaterial->textureIDs[j]);
		}
	}
}

void Tutorial7_AdvTexturingNormMap::LoadTexture(char * filePath)
{
	//  load image data
	int width = 0;
	int height = 0;
	int format = 0;
	unsigned char* pixelData = stbi_load(filePath,
		&width, &height, &format, STBI_default);

	printf("Width: %i Height: %i Format: %i\n", width, height, format);

	// create OpenGL texture handle
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	// set pixel data for texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);

	// set filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// clear bound texture state so we don't accidentally change it
	glBindTexture(GL_TEXTURE_2D, 0);

	// delete pixel data here instead!
	delete[] pixelData;
}

void Tutorial7_AdvTexturingNormMap::CreateShaders()
{
	// load shaders and link shader program
	m_vertShader = Utility::loadShader("shaders/textured.vert", GL_VERTEX_SHADER);
	m_fragShader = Utility::loadShader("shaders/textured.frag", GL_FRAGMENT_SHADER);

	// our vertex buffer has 3 properties per-vertex
	const char* inputs[] = { "position", "colour", "textureCoordinate" };
	m_programID = Utility::createProgram(m_vertShader, 0, 0, 0, m_fragShader, 3, inputs);
}