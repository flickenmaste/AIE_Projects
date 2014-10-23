#include "Tutorial8_Tessellation.h"
#include "Gizmos.h"
#include "Utilities.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

Tutorial8_Tessellation::Tutorial8_Tessellation()
{

}

Tutorial8_Tessellation::~Tutorial8_Tessellation()
{

}

bool Tutorial8_Tessellation::onCreate(int a_argc, char* a_argv[])
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
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Custom code ***

	m_displacementScale = 1;

	Utility::build3DPlane(10, m_VAO, m_VBO, m_IBO);

	CreateTessellation();

	return true;
}

void Tutorial8_Tessellation::onUpdate(float a_deltaTime)
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

	m_displacementScale = std::sin(glfwGetTime()) * 2.5f;

	// quit our application when escape is pressed
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		quit();


}

void Tutorial8_Tessellation::onDraw()
{
	// clear the backbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);

	// draw the gizmos from this frame
	Gizmos::draw(m_projectionMatrix, viewMatrix);

	// bind shader to the GPU
	glUseProgram(m_programID);

	// fetch locations of the view and projection matrices and bind them
	GLuint uProjectionView = glGetUniformLocation(m_programID, "projectionView");
	GLuint uTextureMap = glGetUniformLocation(m_programID, "textureMap");
	GLuint uGlobal = glGetUniformLocation(m_programID, "global");
	GLuint uDisplacement = glGetUniformLocation(m_programID, "displacementMap");
	GLuint uDisplacementScale = glGetUniformLocation(m_programID, "displacementScale");

	// activate texture slot 0 and bind our texture to it
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_displacement);

	// fetch the location of the texture sampler and bind it to 0
	glUniform1i(uTextureMap, 0);
	glUniform1i(uDisplacement, 1);

	// bind other uniforms
	glUniformMatrix4fv(uProjectionView, 1, false, glm::value_ptr(m_projectionMatrix * viewMatrix));
	glUniformMatrix4fv(uGlobal, 1, false, glm::value_ptr(m_global));
	glUniform1f(uDisplacementScale, m_displacementScale);

	// bind 3D plane and draw it
	glBindVertexArray(m_VAO);
	glPatchParameteri(GL_PATCH_VERTICES, 3);
	glDrawElements(GL_PATCHES, 6, GL_UNSIGNED_INT, nullptr);

}

void Tutorial8_Tessellation::onDestroy()
{
	// clean up anything we created
	Gizmos::destroy();

	glDeleteShader(m_vertShader);
	glDeleteShader(m_fragShader);
}

// main that controls the creation/destruction of an application
int main(int argc, char* argv[])
{
	// explicitly control the creation of our application
	Application* app = new Tutorial8_Tessellation();

	if (app->create("AIE - Tutorial8_Tessellation", DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT, argc, argv) == true)
		app->run();

	// explicitly control the destruction of our application
	delete app;

	return 0;
}

void Tutorial8_Tessellation::LoadTexture(char * filePath)
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

void Tutorial8_Tessellation::CreateShaders()
{
	// load shaders and link shader program
	m_vertShader = Utility::loadShader("shaders/textured.vert", GL_VERTEX_SHADER);
	m_fragShader = Utility::loadShader("shaders/textured.frag", GL_FRAGMENT_SHADER);

	// our vertex buffer has 3 properties per-vertex
	const char* inputs[] = { "position", "colour", "textureCoordinate" };
	m_programID = Utility::createProgram(m_vertShader, 0, 0, 0, m_fragShader, 3, inputs);
}

void Tutorial8_Tessellation::CreateTessellation()
{
	int width, height, format;
	unsigned char* data = nullptr;

	data = stbi_load("./textures/rock_diffuse.tga", &width, &height, &format, STBI_default);
	printf("Width: %i Height: %i Format: %i\n", width, height, format);

	// convert the stbi format to the actual GL code
	switch (format)
	{
	case STBI_grey: format = GL_LUMINANCE; break;
	case STBI_grey_alpha: format = GL_LUMINANCE_ALPHA; break;
	case STBI_rgb: format = GL_RGB; break;
	case STBI_rgb_alpha: format = GL_RGBA; break;
	};

	printf("Width: %i Height: %i Format: %i\n", width, height, format);

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	data = stbi_load("./textures/rock_displacement.tga", &width, &height, &format, STBI_default);
	printf("Width: %i Height: %i Format: %i\n", width, height, format);

	// convert the stbi format to the actual GL code
	switch (format)
	{
	case STBI_grey: format = GL_LUMINANCE; break;
	case STBI_grey_alpha: format = GL_LUMINANCE_ALPHA; break;
	case STBI_rgb: format = GL_RGB; break;
	case STBI_rgb_alpha: format = GL_RGBA; break;
	};

	printf("Width: %i Height: %i Format: %i\n", width, height, format);

	glGenTextures(1, &m_displacement);
	glBindTexture(GL_TEXTURE_2D, m_displacement);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	unsigned int vs = Utility::loadShader("./shaders/displace.vert", GL_VERTEX_SHADER);
	unsigned int cs = Utility::loadShader("./shaders/displace.cont", GL_TESS_CONTROL_SHADER);
	unsigned int es = Utility::loadShader("./shaders/displace.eval", GL_TESS_EVALUATION_SHADER);
	unsigned int fs = Utility::loadShader("./shaders/displace.frag", GL_FRAGMENT_SHADER);
	m_programID = Utility::createProgram(vs, cs, es, 0, fs);
	glDeleteShader(vs);
	glDeleteShader(cs);
	glDeleteShader(es);
	glDeleteShader(fs);
}