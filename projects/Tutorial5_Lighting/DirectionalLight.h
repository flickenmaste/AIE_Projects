#pragma once

#include <glm/glm.hpp>

class DirectionalLight
{
public:

	DirectionalLight();
	DirectionalLight(glm::vec3 a_v3Facing, glm::vec3 a_v3Color);
	~DirectionalLight();

	glm::vec3 m_v3Facing;
	glm::vec3 m_v3Color;
	glm::vec3 m_v3SpecularColor;
};