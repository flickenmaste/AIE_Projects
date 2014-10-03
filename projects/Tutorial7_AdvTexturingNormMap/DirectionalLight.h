#ifndef _DIRECTIONALLIGHT_H_
#define _DIRECTIONALLIGHT_H_

#include <glm/glm.hpp>

class DirectionalLight
{
public:
	glm::vec3 Direction;
	glm::vec3 Color;
};

#endif