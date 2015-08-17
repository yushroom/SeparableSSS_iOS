#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>

using namespace std;

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if (p) { delete (p); (p) = NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p); (p) = NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }
#endif
#pragma endregion

void Camera::frameMove(float elapsedTime) {
	angle += angularVelocity * elapsedTime / 150.0f;
	angularVelocity = angularVelocity / (1.0f + attenuation * elapsedTime);
	distance += distanceVelocity * elapsedTime / 150.0f;
	panPosition += panVelocity * elapsedTime / 150.0f;
	build();
}


void Camera::setProjection(float fov, float aspect, float nearPlane, float farPlane) {
	//D3DXMatrixPerspectiveFovLH(&projection, fov, aspect, nearPlane, farPlane);
	//int width, height;
	//glfwGetWindowSize(window, &width, &height);
	//projection = glm::perspectiveFov<float>(fov, width, height, nearPlane, farPlane);
	projection = glm::perspective(fov, aspect, nearPlane, farPlane);
}

bool use_print = false;

void print_m(mat4 const & m)
{
	if (!use_print)
		return;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			printf("%f ", m[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void print_v(vec3 const & v)
{
	if (!use_print)
		return;
	printf("%f %f %f\n", v.x, v.y, v.z);
}

void Camera::build()
{
#if 1
	view = glm::translate(mat4(1.0f), glm::vec3(-panPosition.x, -panPosition.y, -distance));
	view = glm::rotate(view, -angle.y, glm::vec3(1, 0, 0));
	view = glm::rotate(view, -angle.x, glm::vec3(0, 1, 0));

	mat4 viewInverse = glm::inverse(view);
	lookAtPosition = vec3(viewInverse * vec4(0.0f, 0.0f, -distance, 1.0f));
	//print_v(lookAtPosition);
	eyePosition = vec3(viewInverse * vec4(0.0f, 0.0f, 0.0f, 1.0f));
	//print_v(eyePosition);
#else
	mat4 T(1.0f);
	mat4 Rx(1.0f);
	mat4 Ry(1.0f);
	T = glm::translate(mat4(1.0f), glm::vec3(-panPosition.x, -panPosition.y, distance));
	print_m(T);
	Rx = glm::rotate(mat4(1.0f), angle.y, glm::vec3(1, 0, 0));
	print_m(Rx);
	Ry = glm::rotate(mat4(1.0f), angle.x, glm::vec3(0, 1, 0));
	print_m(Ry);
	view = T* Rx * Ry;
	print_m(view);

	//view = glm::scale(view, glm::vec3(1, 1, -1));
	mat4 viewInverse = glm::inverse(view);

	lookAtPosition = vec3(viewInverse * vec4(0.0f, 0.0f, distance, 1.0f));
	print_v(lookAtPosition);

	eyePosition = vec3(viewInverse * vec4(0.0f, 0.0f, 0.0f, 1.0f));
	print_v(eyePosition);

	lookAtPosition.z = -lookAtPosition.z;
	eyePosition.z = -eyePosition.z;
	view = glm::lookAtRH(eyePosition, lookAtPosition, vec3(0, 1, 0));

	viewInverse = glm::inverse(view);
	lookAtPosition = vec3(viewInverse * vec4(0.0f, 0.0f, -distance, 1.0f));
	print_v(lookAtPosition);
	eyePosition = vec3(viewInverse * vec4(0.0f, 0.0f, 0.0f, 1.0f));
	print_v(eyePosition);

#endif

}


void Camera::updatePosition(vec2 delta) {
	delta.x /= viewportSize.x / 2.0f;
	delta.y /= viewportSize.y / 2.0f;

	mat4 transform = mat4(1.0f);
	transform = glm::translate(transform, vec3(0.0f, 0.0f, distance));
	transform = projection * transform;

	mat4 inverse = glm::inverse(transform);

	vec4 t = vec4(panPosition.x, panPosition.y, 0.0f, 1.0f);
	t = transform * t;
	t.x -= delta.x * t.w;
	t.y -= delta.y * t.w;
	t = inverse * t;
	panPosition = vec2(t);
}


ostream &operator <<(ostream &os, const Camera &camera) {
	os << camera.distance << endl;
	os << camera.angle.x << " " << camera.angle.y << endl;
	os << camera.panPosition.x << " " << camera.panPosition.y << endl;
	os << camera.angularVelocity.x << " " << camera.angularVelocity.y << endl;
	return os;
}


istream &operator >>(istream &is, Camera &camera) {
	is >> camera.distance;
	is >> camera.angle.x >> camera.angle.y;
	is >> camera.panPosition.x >> camera.panPosition.y;
	is >> camera.angularVelocity.x >> camera.angularVelocity.y;
	return is;
}
