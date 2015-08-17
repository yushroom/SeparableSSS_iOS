#ifndef RENDERCONTEX_H
#define RENDERCONTEX_H

#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Shader.h"


class RenderContex
{
public:

	static void set_window_size(int width, int height)
	{
		window_width = width;
		window_height = height;
	}

	static int window_width;
	static int window_height;

	static glm::mat4 model_mat;
	static glm::mat4 prev_mvp;
	static Camera* camera;

	static glm::mat4 get_model_inverse_transpose()
	{
		return glm::transpose(glm::inverse(model_mat));
	}

	static glm::mat4 get_mvp_mat()
	{
		return camera->getProjectionMatrix() * camera->getViewMatrix() * model_mat;
	}

private:
	RenderContex();
};


#endif
