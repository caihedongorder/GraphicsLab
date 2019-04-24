#pragma once
#include "Transform.h"

class Camera
{
public:
	Camera();
	~Camera();

	void Init(const glm::vec3& EyeAt, const glm::vec3& TargetPos,
		float InFovy,float InAr,float InNearPlane,float InFarPlane);

	glm::mat4 GetProjectMat() { return ProjectMat; }
	glm::mat4 GetViewMat() { return ViewMat; }


	void Translate(const glm::vec3& InDelta);

private:
	Transform mTransform;

	float mFovy;
	float mAr;
	float mNearPlane;
	float mFarPlane;

	glm::mat4 ProjectMat;
	glm::mat4 ViewMat;
};

