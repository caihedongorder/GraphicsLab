#pragma once
#include "Transform.h"
#include <glm/gtx/quaternion.hpp>


class MathUtil
{
public:
	MathUtil();
	~MathUtil();

	static glm::mat4 transform(const Transform& InTransform);
	static glm::mat4 transformNoScale(const Transform& InTransform);


	static glm::mat4 InverseTransform(const Transform& InTransform);
	static glm::mat4 InverseTransformNoScale(const Transform& InTransform);

	static glm::quat MakeQuad(const glm::mat3& InMat);
	static glm::quat MakeQuad(const glm::vec3& InRotateAxis, float InRotateAngle);
	static glm::quat MakeQuad(const glm::vec3& InStartVec, const glm::vec3& InDestVec);
	static glm::quat MakeQuadFromXY(const glm::vec3& InXAxis, const glm::vec3& InYAxis);
	static glm::quat MakeQuadFromXZ(const glm::vec3& InXAxis, const glm::vec3& InZAxis);
	static glm::quat MakeQuadFromYZ(const glm::vec3& InYAxis, const glm::vec3& InZAxis);
	static glm::quat MakeQuadFromX(const glm::vec3& InXAxis);
	static glm::quat MakeQuadFromY(const glm::vec3& InYAxis);
	static glm::quat MakeQuadFromZ(const glm::vec3& InZAxis);
	static glm::quat MakeQuadByEulerAngle(const glm::vec3& InEulerAngle);
	static glm::mat4 MakeMat4(const glm::quat& InQuat);
	static bool QuatsIsNearEqual(const glm::quat& InQuat1, const glm::quat& InQuat2);

	static glm::vec3 rotateVector(const glm::quat& InQuat, const glm::vec3& InVector);

	static glm::quat SLerp(const glm::quat& InQuat1, const glm::quat& InQuat2, float InAlpha);

	/*
		无论是矩形还是 3d的aabb 都是检测每条轴的相交情况
	*/
	static bool TwoRectIntersect(const glm::vec4& InRect1, const glm::vec4& InRect2, glm::vec4& OutRect);
};

