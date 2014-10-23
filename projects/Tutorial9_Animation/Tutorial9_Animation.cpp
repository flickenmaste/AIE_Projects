#include "Tutorial9_Animation.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

Tutorial9_Animation::Tutorial9_Animation()
{

}

Tutorial9_Animation::~Tutorial9_Animation()
{

}

bool Tutorial9_Animation::onCreate(int a_argc, char* a_argv[])
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
	glEnable(GL_CULL_FACE);

	// Custom code ***

	// load the shader
	const char* aszInputs[] = { "Position", "TexCoord", "indices", "weights"};
	const char* aszOutputs[] = { "outColour" };

	// load shader internally calls glCreateShader...
	GLuint vshader = Utility::loadShader("./shaders/SimpleFBX_Tutorial.vert", GL_VERTEX_SHADER);
	GLuint pshader = Utility::loadShader("./shaders/SimpleFBX_Tutorial.frag", GL_FRAGMENT_SHADER);

	m_shader = Utility::createProgram(vshader, 0, 0, 0, pshader, 4, aszInputs, 1, aszOutputs);

	// free our shader once we built our program
	glDeleteShader(vshader);
	glDeleteShader(pshader);

	m_fbx = new FBXFile();
	m_fbx->load("./characters/Pyro/pyro.fbx", FBXFile::UNITS_METER);
	m_fbx->initialiseOpenGLTextures();
	InitFBXSceneResource(m_fbx);

	return true;
}

void Tutorial9_Animation::onUpdate(float a_deltaTime)
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

	// grab the skeleton and animation we want to use
	FBXSkeleton* skeleton = m_fbx->getSkeletonByIndex(0);
	FBXAnimation* animation = m_fbx->getAnimationByIndex(0);

	// evaluate the animation to update bones
	skeleton->evaluate(animation, Utility::getTotalTime());

	UpdateFBXSceneResource(m_fbx);
}

void Tutorial9_Animation::onDraw()
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

void Tutorial9_Animation::onDestroy()
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
	Application* app = new Tutorial9_Animation();

	if (app->create("AIE - Tutorial9_Animation", DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT, argc, argv) == true)
		app->run();

	// explicitly control the destruction of our application
	delete app;

	return 0;
}

void Tutorial9_Animation::InitFBXSceneResource(FBXFile *a_pScene)
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
		glEnableVertexAttribArray(0); // position
		glEnableVertexAttribArray(1); // texture coordinate
		glEnableVertexAttribArray(2); // indices
		glEnableVertexAttribArray(3); // blend weights

		// tell our shaders where the information within our buffers lie
		// eg: attrubute 0 is expected to be the verticy's position. it should be 4 floats, representing xyzw
		// eg: attrubute 1 is expected to be the verticy's color. it should be 4 floats, representing rgba
		// eg: attrubute 2 is expected to be the verticy's texture coordinate. it should be 2 floats, representing U and V
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::PositionOffset);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::TexCoord1Offset);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::IndicesOffset);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(FBXVertex), (char*)FBXVertex::WeightsOffset);

		// finally, where done describing our mesh to the shader
		// we can describe the next mesh
		glBindVertexArray(0);
	}

}

void Tutorial9_Animation::UpdateFBXSceneResource(FBXFile *a_pScene)
{
	a_pScene->getRoot()->updateGlobalTransform();
}

void Tutorial9_Animation::RenderFBXSceneResource(FBXFile *a_pScene, glm::mat4 a_view, glm::mat4 a_projection)
{
	// activate a shader
	glUseProgram(m_shader);

	// update the bones to include the bind pose
	// so that the offset is local
	FBXSkeleton* skeleton = m_fbx->getSkeletonByIndex(0);
	skeleton->updateBones();

	// get the location of uniforms on the shader
	GLint uDiffuseTexture = glGetUniformLocation(m_shader, "DiffuseTexture");
	GLint uDiffuseColor = glGetUniformLocation(m_shader, "DiffuseColor");

	GLint uProjectionView = glGetUniformLocation(m_shader, "projectionView");
	GLint uGlobal = glGetUniformLocation(m_shader, "global");
	GLint uLocation = glGetUniformLocation(m_shader, "bones");

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


		// reset back to the default active texture
		glActiveTexture(GL_TEXTURE0);

		// tell the shader which texture to use
		glUniform1i(uDiffuseTexture, 1);

		m_projectionView = a_projection * a_view;

		// send the Model, View and Projection Matrices to the shader
		glUniformMatrix4fv(uProjectionView, 1, false, glm::value_ptr(m_projectionView));
		glUniformMatrix4fv(uGlobal, 1, false, glm::value_ptr(mesh->m_globalTransform));
		glUniformMatrix4fv(uLocation, skeleton->m_boneCount, GL_FALSE, (float*)skeleton->m_bones);

		// bind our vertex array object
		// remember in the initialise function, we bound the VAO and IBO to the VAO
		// so when we bind the VAO, openGL knows what what vertices,
		// indices and vertex attributes to send to the shader
		glBindVertexArray(ro->VAO);
		glDrawElements(GL_TRIANGLES, mesh->m_indices.size(), GL_UNSIGNED_INT, 0);

	}

	// finally, we have finished rendering the meshes
	// disable this shader
	glUseProgram(0);
}

void Tutorial9_Animation::DestroyFBXSceneResource(FBXFile *a_pScene)
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