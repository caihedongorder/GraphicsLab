#pragma once

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/gtc/constants.hpp> // glm::pi
#include <glm/gtc/quaternion.hpp>

struct Transform
{
	glm::vec3 location;
	glm::quat Quat;
	glm::vec3 scale;

	void Translate(const glm::vec3& InDelta);
};

