#include "SDL.h"
#include "SSSSRender.h"
#include <time.h>

#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>

#include <memory>
using namespace std;

#include "GLError.h"


int main(int argc, char *argv[])
{
    
    SDL_Window *window;
    //SDL_Renderer *renderer;
    int done;
    SDL_Event event;
    
    /* initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Could not initialize SDL\n");
        return 1;
    }
    
    /* seed random number generator */
    srand(time(NULL));
    
    SDL_DisplayMode displayMode;
    SDL_GetDesktopDisplayMode(0, &displayMode);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    
    /* create window and renderer */
    window = SDL_CreateWindow(NULL, 0, 0, displayMode.w, displayMode.h, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        printf("Could not initialize Window\n");
        return 1;
    }
    
    SDL_GL_CreateContext(window);
    
    unique_ptr<SSSSRender> render = unique_ptr<SSSSRender>(new SSSSRender(window));
    if ( ! render->initialize())
    {
        return -1;
    }
    
    float current_time = SDL_GetTicks();
    float old_time = 0;
    float delta_time = 0;
    
    /* Enter render loop, waiting for user to quit */
    done = 0;
    while (!done)
    {
        old_time = current_time;
        current_time = SDL_GetTicks();
        delta_time = (current_time - old_time) / 1000.f;
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
            }
        }

        render->update(delta_time);
        render->render();
        
        check_gl_error();
        SDL_GL_SwapWindow(window);
        SDL_Delay(1);
    }
    
    /* shutdown SDL */
    SDL_Quit();
    
    return 0;
}
