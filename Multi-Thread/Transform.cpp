#include "stdafx.h"
#include "Transform.h"

void Transform::Translate(const glm::vec3& InDelta)
{
	location += InDelta;
}
