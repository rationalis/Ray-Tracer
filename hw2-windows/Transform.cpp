// Transform.cpp: implementation of the Transform class.

// Note: when you construct a matrix using mat4() or mat3(), it will be COLUMN-MAJOR
// Keep this in mind in readfile.cpp and display.cpp
// See FAQ for more details or if you're having problems.

#include "Transform.h"

// Helper rotation function.  
mat3 Transform::rotate(const float degrees, const vec3& axis) {
	mat3 I(1);
	float x = axis[0];
	float y = axis[1];
	float z = axis[2];
	mat3 aat(x * axis, y * axis, z * axis);
	mat3 astar(0, z, -y, -z, 0, x, y, -x, 0);
	float angle = glm::radians(degrees);
	float cosine = cos(angle);
	float sine = sin(angle);
	return cosine*I + (1.0f - cosine)*aat + sine*astar;
}

// Transforms the camera left around the "crystal ball" interface
void Transform::left(float degrees, vec3& eye, vec3& up) {
	eye = rotate(degrees, upvector(up,eye)) * eye;
}

// Transforms the camera up around the "crystal ball" interface
void Transform::up(float degrees, vec3& eye, vec3& up) {
	vec3 a = glm::normalize(glm::cross(eye, up));
	eye = rotate(degrees, a) * eye;
	up = rotate(degrees, a) * up;
}

mat4 Transform::lookAt(const vec3 &eye, const vec3 &center, const vec3 &up) 
{
    mat4 ret;
	vec3 w = glm::normalize(eye - center);
	vec3 u = glm::normalize(glm::cross(up, w));
	vec3 v = glm::cross(w, u);
	mat3 R3(u, v, w);
	R3 = glm::transpose(R3);
	mat4 R(R3);
	mat4 T(1);
	T[3][0] = -eye[0];
	T[3][1] = -eye[1];
	T[3][2] = -eye[2];
	return R*T;
}

mat4 Transform::perspective(float fovy, float aspect, float zNear, float zFar)
{
	const float d = cos(glm::radians(fovy) / 2) / sin(glm::radians(fovy) / 2);
	const float A = -(zFar + zNear) / (zFar - zNear);
	const float B = -(2 * zFar * zNear) / (zFar - zNear);
    mat4 ret(
		d/aspect, 0,0,0,
		0,d,0,0,
		0,0,A,-1,
		0,0,B,0);
    return ret;
}

mat4 Transform::scale(const float &sx, const float &sy, const float &sz) 
{
	mat3 scale(sx, 0, 0, 0, sy, 0, 0, 0, sz);
    mat4 ret(scale);
    return ret;
}

mat4 Transform::translate(const float &tx, const float &ty, const float &tz) 
{
	mat4 T(1);
	T[3][0] = tx;
	T[3][1] = ty;
	T[3][2] = tz;
    return T;
}

// To normalize the up direction and construct a coordinate frame.  
// As discussed in the lecture.  May be relevant to create a properly 
// orthogonal and normalized up. 
// This function is provided as a helper, in case you want to use it. 
// Using this function (in readfile.cpp or display.cpp) is optional.  

vec3 Transform::upvector(const vec3 &up, const vec3 & zvec) 
{
    vec3 x = glm::cross(up,zvec); 
    vec3 y = glm::cross(zvec,x); 
    vec3 ret = glm::normalize(y); 
    return ret; 
}


Transform::Transform()
{

}

Transform::~Transform()
{

}
