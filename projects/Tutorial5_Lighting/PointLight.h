#pragma once

#include <glm/glm.hpp>

class PointLight
{
public:

	PointLight();
	PointLight(glm::vec3 a_v3Position, glm::vec3 a_v3Color);
	~PointLight();

	glm::vec3 m_v3Position;
	glm::vec3 m_v3Color;
	glm::vec3 m_v3SpecularColor;
};