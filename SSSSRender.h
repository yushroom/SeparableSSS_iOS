//
//  SSSSRender.h
//  SeparableSSS
//
//  Created by yushroom on 8/13/15.
//
//

#ifndef SeparableSSS_SSSSRender_h
#define SeparableSSS_SSSSRender_h

#include <string>
#include <CoreFoundation/CoreFoundation.h>

#include <glm/gtc/matrix_transform.hpp>

#include "SDL.h"
#include "Shader.h"
#include "Model.h"
#include "Debug.h"
#include "Camera.h"
#include "RenderContex.h"
#include "TextureLoader.h"

#define CAMERA_FOV 20.0f
#define PI 3.1415926536f

using std::string;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

static string IOS_bundle_path( CFStringRef subDir, CFStringRef name, CFStringRef ext)
{
    CFURLRef url = CFBundleCopyResourceURL(CFBundleGetMainBundle(), name, ext, subDir);
    UInt8 path[1024];
    CFURLGetFileSystemRepresentation(url, true, path, sizeof(path));
    CFRelease(url);
    return string((const char*)path);
}
#define IOS_PATH(subDir, name, ext) IOS_bundle_path(CFSTR(subDir), CFSTR(name), CFSTR(ext))


class SSSSRender
{
public:
    SSSSRender(SDL_Window * window) : _window(window)
    {
        SDL_GetWindowSize(_window, &_width, &_height);
        
        RenderContex::set_window_size(_width, _height);
    }
    
    bool initialize()
    {
        _shader_skydome.init(IOS_PATH("shader", "SkyDome", "vert"), IOS_PATH("shader", "SkyDome", "frag"));
        simple_shader.init(IOS_PATH("shader", "simple", "vert"), IOS_PATH("shader", "simple", "frag"));
        
        _tex_sky = TextureLoader::CreateTextureCubemap(IOS_PATH("StPeters", "DiffuseMap", "dds").c_str());
        
        _model_head.init(IOS_PATH("head", "head_optimized", "obj"), true, true, true, false);
        _model_sphere.init(IOS_PATH("StPeters", "Sphere", "obj"));
        
        _camera.setDistance(3.1f);
        _camera.setAngle(vec2(-0.49005f, 0.0508272f));
        _camera.setPanPosition(vec2(-0.0168704, 0.0729295));
        _camera.setAngularVelocity(vec2(0, 0));
        _camera.setProjection(CAMERA_FOV * PI / 180.0f, 1.0f * _width / _height, 0.1f, 100.0f);
        _camera.build();
        
        init_opengl();
        
        test_mat4 = glm::perspective(CAMERA_FOV * PI / 180.0f, 1.0f * _width / _height, 0.1f, 100.0f) * glm::lookAt(vec3(0, 0, 0), vec3(0, 0, -1), vec3(0, 1, 0));
        
        return true;
    }
    
    void init_opengl()
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        //glStencilFunc(GL_KEEP, GL_KEEP, GL_REPLACE);
        
        glClearColor(0, 1, 0, 0);
    }
    
    void update(float t)
    {
        _camera.frameMove(t);
    }
    
    void main_pass()
    {
        glDisable(GL_CULL_FACE);
        RenderContex::model_mat = glm::scale(mat4(1.0f), vec3(5.0f));
        //glEnable(GL_CULL_FACE);
        //glCullFace(GL_BACK);
        //glCullFace(GL_FRONT);
        RenderContex::camera = &_camera;
        _shader_skydome.use();
        //_shader_skydome.bind_mat4("MVP", RenderContex::get_mvp_mat());
        _shader_skydome.bind_mat4("MVP", test_mat4);
        _shader_skydome.bind_texture("skyTex", _tex_sky, 0, GL_TEXTURE_CUBE_MAP);
        _model_sphere.render();
        //glCullFace(GL_BACK);
    }
    
    void shadow_pass()
    {
        
    }
    
    void render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, _width, _height);
        main_pass();
    }
    
    void shut_down()
    {
        
    }
    
private:
    SDL_Window * _window;
    int _width;
    int _height;
    
    Camera _camera;
    
    // resources
    Shader _shader_skydome;
    Shader _shader_main;
    Shader simple_shader;
    
    Model _model_head;
    Model _model_sphere;
    
    GLuint _tex_sky;
    
    mat4 test_mat4;
};


#endif
