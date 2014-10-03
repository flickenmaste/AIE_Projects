#include "Tutorial7_CubeMap.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

Tutorial7_CubeMap::Tutorial7_CubeMap()
{

}

Tutorial7_CubeMap::~Tutorial7_CubeMap()
{

}

bool Tutorial7_CubeMap::onCreate(int a_argc, char* a_argv[])
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

	CreateCubeMapTex();
	CreateCubeMapBuffers();
	CreateCubeMapShaders();

	return true;
}

void Tutorial7_CubeMap::onUpdate(float a_deltaTime)
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

	//UpdateFBXSceneResource(m_fbx);

	//if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS) m_decayValue -= a_deltaTime;
	//if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS) m_decayValue += a_deltaTime;

	//if (m_decayValue < 0.0f) m_decayValue = 0.0f;
	//if (m_decayValue > 1.0f) m_decayValue = 1.0f;
}

void Tutorial7_CubeMap::onDraw()
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
	//RenderFBXSceneResource(m_fbx, viewMatrix, m_projectionMatrix);

	// draw the skybox
	glUseProgram(m_skybox_shader);

	glm::mat4 MVP = m_projectionMatrix * viewMatrix * glm::scale(glm::vec3(5, 5, 5));

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_texture_id);

	glUniformMatrix4fv(glGetUniformLocation(m_skybox_shader, "ModelViewProjection"), 1, false, glm::value_ptr(MVP));
	glUniform1i(glGetUniformLocation(m_skybox_shader, "CubeMap"), 1);

	glBindVertexArray(m_skybox_vao);
	glDrawElements(GL_TRIANGLES, 72, GL_UNSIGNED_SHORT, 0);
}

void Tutorial7_CubeMap::onDestroy()
{
	// clean up anything we created
	Gizmos::destroy();
}

// main that controls the creation/destruction of an application
int main(int argc, char* argv[])
{
	// explicitly control the creation of our application
	Application* app = new Tutorial7_CubeMap();

	if (app->create("AIE - Tutorial7_CubeMap", DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT, argc, argv) == true)
		app->run();

	// explicitly control the destruction of our application
	delete app;

	return 0;
}

void Tutorial7_CubeMap::InitFBXSceneResource(FBXFile *a_pScene)
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

		// tell our shaders where the information within our buffers lie
		// eg: attrubute 0 is expected to be the verticy's position. it should be 4 floats, representing xyzw
		// eg: attrubute 1 is expected to be the verticy's color. it should be 4 floats, representing rgba
		// eg: attrubute 2 is expected to be the verticy's texture coordinate. it should be 2 floats, representing U and V
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char *)FBXVertex::PositionOffset);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char *)FBXVertex::ColourOffset);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char *)FBXVertex::TexCoord1Offset);

		// finally, where done describing our mesh to the shader
		// we can describe the next mesh
		glBindVertexArray(0);
	}

}

void Tutorial7_CubeMap::UpdateFBXSceneResource(FBXFile *a_pScene)
{
	a_pScene->getRoot()->updateGlobalTransform();
}

void Tutorial7_CubeMap::RenderFBXSceneResource(FBXFile *a_pScene, glm::mat4 a_view, glm::mat4 a_projection)
{
	// enable transparent blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// activate a shader
	glUseProgram(m_shader);

	// get the location of uniforms on the shader
	GLint uDiffuseTexture = glGetUniformLocation(m_shader, "DiffuseTexture");
	GLint uDiffuseColor = glGetUniformLocation(m_shader, "DiffuseColor");

	GLint uModel = glGetUniformLocation(m_shader, "Model");
	GLint uView = glGetUniformLocation(m_shader, "View");
	GLint uProjection = glGetUniformLocation(m_shader, "Projection");

	GLint uDecayTexture = glGetUniformLocation(m_shader, "DecayTexture");
	GLint uDecayValue = glGetUniformLocation(m_shader, "DecayValue");

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
		glBindTexture(GL_TEXTURE_2D, m_texture);

		// tell the shader which texture to use
		glUniform1i(uDiffuseTexture, 1);
		glUniform1i(uDecayTexture, 2);

		// tell the shader the decay value
		glUniform1f(uDecayValue, m_decayValue);

		// send the Model, View and Projection Matrices to the shader
		glUniformMatrix4fv(uModel, 1, false, glm::value_ptr(mesh->m_globalTransform));
		glUniformMatrix4fv(uView, 1, false, glm::value_ptr(a_view));
		glUniformMatrix4fv(uProjection, 1, false, glm::value_ptr(a_projection));

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

void Tutorial7_CubeMap::DestroyFBXSceneResource(FBXFile *a_pScene)
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

void Tutorial7_CubeMap::LoadTexture(char * filePath)
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

void Tutorial7_CubeMap::CreateShaders()
{
	// load shaders and link shader program
	m_vertShader = Utility::loadShader("shaders/textured.vert", GL_VERTEX_SHADER);
	m_fragShader = Utility::loadShader("shaders/textured.frag", GL_FRAGMENT_SHADER);

	// our vertex buffer has 3 properties per-vertex
	const char* inputs[] = { "position", "colour", "textureCoordinate" };
	m_programID = Utility::createProgram(m_vertShader, 0, 0, 0, m_fragShader, 3, inputs);
}

void Tutorial7_CubeMap::CreateCubeMapTex()
{
	// cube mapping must be enabled
	glEnable(GL_TEXTURE_CUBE_MAP);

	// genorate an  OpenGL texture and bind it as a GL_TEXTURE_CUBE_MAP
	glGenTextures(1, &m_cubemap_texture_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap_texture_id);

	// set some texture parameters...
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// temporary width and height variables
	int width, height, format;

	// get the pixels for each of face of the cube and send the data to the graphics card
	unsigned char *pixels_top = stbi_load("./textures/cubemap/top.png", &width, &height, &format, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_top);

	unsigned char *pixels_bottom = stbi_load("./textures/cubemap/bottom.png", &width, &height, &format, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_bottom);

	unsigned char *pixels_north = stbi_load("./textures/cubemap/north.png", &width, &height, &format, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_north);

	unsigned char *pixels_south = stbi_load("./textures/cubemap/south.png", &width, &height, &format, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_south);

	unsigned char *pixels_east = stbi_load("./textures/cubemap/east.png", &width, &height, &format, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_east);

	unsigned char *pixels_west = stbi_load("./textures/cubemap/west.png", &width, &height, &format, 0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_west);

	// delete the original image data
	free(pixels_top);
	free(pixels_bottom);
	free(pixels_north);
	free(pixels_south);
	free(pixels_east);
	free(pixels_west);

}

void Tutorial7_CubeMap::CreateCubeMapBuffers()
{
	// Genorate our cube
	// cube vertices for vertex buffer object
	float cube_vertices[] =
	{
		//	x,	  y,	z
		-1.0, 1.0, 1.0,
		-1.0, -1.0, 1.0,
		1.0, -1.0, 1.0,
		1.0, 1.0, 1.0,
		-1.0, 1.0, -1.0,
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0, 1.0, -1.0,
	};

	unsigned short cube_indices[] =
	{
		0, 1, 2, 0, 2, 3,
		3, 2, 6, 3, 6, 7,
		7, 6, 5, 7, 5, 4,
		4, 5, 1, 4, 1, 0,
		0, 3, 7, 0, 7, 4,
		1, 2, 6, 1, 6, 5,
	};

	// OPENGL: genorate the VBO, IBO and VAO
	glGenBuffers(1, &m_skybox_vbo);
	glGenBuffers(1, &m_skybox_ibo);
	glGenVertexArrays(1, &m_skybox_vao);

	// send the data
	glBindVertexArray(m_skybox_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_skybox_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_skybox_ibo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

	// setup vertex attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);
}

void Tutorial7_CubeMap::CreateCubeMapShaders()
{
	// load the skybox shader
	const char* aszInputs[] = { "Position" };
	const char* aszOutputs[] = { "outColour" };

	// load shader internally calls glCreateShader...
	GLuint vshader = Utility::loadShader("./shaders/cubeMap.vert", GL_VERTEX_SHADER);
	GLuint pshader = Utility::loadShader("./shaders/cubeMap.frag", GL_FRAGMENT_SHADER);

	m_skybox_shader = Utility::createProgram(vshader, 0, 0, 0, pshader, 1, aszInputs, 1, aszOutputs);
}