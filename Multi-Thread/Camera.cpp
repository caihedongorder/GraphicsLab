#include "stdafx.h"
#include "Camera.h"
#include "MathUtil.h"


Camera::Camera()
{
}


Camera::~Camera()
{
}

void Camera::Init(const glm::vec3& EyeAt, const glm::vec3& TargetPos,
	float InFovy, float InAr, float InNearPlane, float InFarPlane)
{
	const glm::vec3 Up = glm::vec3(0, 1, 0);

	mTransform.location = EyeAt;
	mTransform.scale = glm::vec3(1, 1, 1);
	
	auto lookDir = glm::normalize(TargetPos - EyeAt);

	mTransform.Quat =  MathUtil::MakeQuadFromZ(lookDir);

	mFovy = InFovy;
	mAr = InAr;
	mNearPlane = InNearPlane;
	mFarPlane = InFarPlane;

	ProjectMat = glm::perspectiveLH(glm::radians(InFovy), InAr, InNearPlane, InFarPlane);


	Translate({ 0.0f,0.0f,0.0f });

	ViewMat = MathUtil::InverseTransformNoScale(mTransform);
}

void Camera::Translate(const glm::vec3& InDelta)
{
	mTransform.Translate(InDelta);

}
