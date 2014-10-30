#include "PointLight.h"

PointLight::PointLight()
{
	m_v3Position = glm::vec3(0, -1, 0);
	m_v3Color = glm::vec3(1, 1, 1);
	m_v3SpecularColor = glm::vec3(1, 1, 1);
}

PointLight::PointLight(glm::vec3 a_v3Position, glm::vec3 a_v3Color)
{
	m_v3Position = a_v3Position;
	m_v3Color = a_v3Color;
	m_v3SpecularColor = a_v3Color;
}

PointLight::~PointLight()
{

}
