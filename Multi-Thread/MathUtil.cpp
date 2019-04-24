#include "stdafx.h"
#include "MathUtil.h"


MathUtil::MathUtil()
{
}


MathUtil::~MathUtil()
{
}

glm::mat4 MathUtil::transform(const Transform& InTransform)
{
	glm::mat4 Mat = glm::toMat4(InTransform.Quat);
	Mat[0] *= InTransform.scale[0];
	Mat[1] *= InTransform.scale[1];
	Mat[2] *= InTransform.scale[2];

	Mat[3] = glm::vec4(InTransform.location, 0);

	return Mat;
}

glm::mat4 MathUtil::transformNoScale(const Transform& InTransform)
{
	glm::mat4 Mat = glm::toMat4(InTransform.Quat);
	Mat[3] = glm::vec4(InTransform.location, 0);
	return Mat;
}

glm::mat4 MathUtil::InverseTransform(const Transform& InTransform)
{

	glm::mat3 RotateMat = glm::toMat3(InTransform.Quat);

	auto InvQ = glm::transpose(RotateMat);
	glm::mat3 InvS(1.0f);
	InvS[0][0] = 1.0f / InTransform.scale[0];
	InvS[1][1] = 1.0f / InTransform.scale[1];
	InvS[2][2] = 1.0f / InTransform.scale[2];

	auto InvQS = InvQ * InvS;

	glm::mat4 mat = InvQS;

	mat[3][0] = -dot(glm::vec3(mat[0][0], mat[1][0], mat[2][0]), InTransform.location);
	mat[3][1] = -dot(glm::vec3(mat[0][1], mat[1][1], mat[2][1]), InTransform.location);
	mat[3][2] = -dot(glm::vec3(mat[0][2], mat[1][2], mat[2][2]), InTransform.location);

	return mat;
}

glm::mat4 MathUtil::InverseTransformNoScale(const Transform& InTransform)
{
	glm::mat3 RotateMat = glm::toMat3(InTransform.Quat);

	auto InvQ = glm::transpose(RotateMat);

	glm::mat4 mat = InvQ;

	mat[3][0] = -dot(glm::vec3(mat[0][0], mat[1][0], mat[2][0]), InTransform.location);
	mat[3][1] = -dot(glm::vec3(mat[0][1], mat[1][1], mat[2][1]), InTransform.location);
	mat[3][2] = -dot(glm::vec3(mat[0][2], mat[1][2], mat[2][2]), InTransform.location);

	return mat;
}

glm::quat MathUtil::MakeQuad(const glm::vec3& InRotateAxis, float InRotateAngle)
{
#if 0
	glm::quat q;
	
	float HalfAngle = glm::radians(InRotateAngle) * 0.5f;

	float sin_half_Angle = sinf(HalfAngle);
	q.x = InRotateAxis.x *sin_half_Angle;
	q.y = InRotateAxis.y *sin_half_Angle;
	q.z = InRotateAxis.z *sin_half_Angle;

	q.w = cosf(HalfAngle);
#endif

	return glm::angleAxis(glm::radians(InRotateAngle), InRotateAxis);
}

glm::quat MathUtil::MakeQuad(const glm::vec3& InStartVec, const glm::vec3& InDestVec)
{
	auto start = glm::normalize(InStartVec);
	auto dest = glm::normalize(InDestVec);

	float cosTheta = dot(start, dest);
	glm::vec3 rotationAxis;

	if (cosTheta < -1 + 0.001f) {
		// special case when vectors in opposite directions:
		// there is no "ideal" rotation axis
		// So guess one; any will do as long as it's perpendicular to start
		rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
		if (glm::length2(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
			rotationAxis = cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

		rotationAxis = normalize(rotationAxis);
		return glm::angleAxis(glm::radians(180.0f), rotationAxis);
	}

	rotationAxis = glm::cross(start, dest);

	float s = glm::sqrt((1 + cosTheta) * 2);
	float invs = 1 / s;

	return glm::quat(
		s * 0.5f,
		rotationAxis.x * invs,
		rotationAxis.y * invs,
		rotationAxis.z * invs
	);
}

glm::quat MathUtil::MakeQuad(const glm::mat3& InMat)
{
	return glm::quat_cast(InMat);
}

glm::quat MathUtil::MakeQuadFromXY(const glm::vec3& InXAxis, const glm::vec3& InYAxis)
{
	glm::mat3 mat;

	auto xAxis = glm::normalize(InXAxis);
	auto yAxis = glm::normalize(InYAxis);

	auto zAxis = glm::cross(xAxis, yAxis);

	mat[0] = xAxis;
	mat[1] = yAxis;
	mat[2] = zAxis;

	return MathUtil::MakeQuad(mat);
}

glm::quat MathUtil::MakeQuadFromXZ(const glm::vec3& InXAxis, const glm::vec3& InZAxis)
{

	glm::mat3 mat;

	auto xAxis = glm::normalize(InXAxis);
	auto zAxis = glm::normalize(InZAxis);

	auto yAxis = glm::cross(zAxis, xAxis);

	mat[0] = xAxis;
	mat[1] = yAxis;
	mat[2] = zAxis;

	return MathUtil::MakeQuad(mat);
}

glm::quat MathUtil::MakeQuadFromYZ(const glm::vec3& InYAxis, const glm::vec3& InZAxis)
{
	glm::mat3 mat;

	auto yAxis = glm::normalize(InYAxis);
	auto zAxis = glm::normalize(InZAxis);

	auto xAxis = glm::cross(yAxis, zAxis);

	mat[0] = xAxis;
	mat[1] = yAxis;
	mat[2] = zAxis;

	return MathUtil::MakeQuad(mat);
}

glm::quat MathUtil::MakeQuadFromX(const glm::vec3& InXAxis)
{
	return MathUtil::MakeQuad(glm::vec3(1.0f, 0.0f, 0.0f), InXAxis);
}

glm::quat MathUtil::MakeQuadFromY(const glm::vec3& InYAxis)
{
	return MathUtil::MakeQuad(glm::vec3(0.0f, 1.0f, 0.0f), InYAxis);
}

glm::quat MathUtil::MakeQuadFromZ(const glm::vec3& InZAxis)
{
	return MathUtil::MakeQuad(glm::vec3(0.0f, 0.0f, 1.0f), InZAxis);
}

glm::quat MathUtil::MakeQuadByEulerAngle(const glm::vec3& InEulerAngle)
{
	return glm::quat(glm::vec3(glm::radians(InEulerAngle.x), glm::radians(InEulerAngle.y), glm::radians(InEulerAngle.z)));
}

glm::mat4 MathUtil::MakeMat4(const glm::quat& InQuat)
{
	return glm::toMat4(InQuat);
}

bool MathUtil::QuatsIsNearEqual(const glm::quat& InQuat1, const glm::quat& InQuat2)
{
	return glm::abs(glm::dot(InQuat1, InQuat2)) < 0.001f;
}

glm::vec3 MathUtil::rotateVector(const glm::quat& InQuat, const glm::vec3& InVector)
{
	return InQuat * InVector;
}

glm::quat MathUtil::SLerp(const glm::quat& InQuat1, const glm::quat& InQuat2, float InAlpha)
{
	return glm::mix(InQuat1, InQuat2, InAlpha);
}
