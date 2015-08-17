#include "RenderContex.h"

int RenderContex::window_width = 0;
int RenderContex::window_height = 0;
glm::mat4 RenderContex::model_mat;
Camera* RenderContex::camera;
glm::mat4 RenderContex::prev_mvp;