//
//  Shader.h
//  NormalMap
//
//  Created by yushroom on 5/6/15.
//
//

#ifndef NormalMap_Shader_h
#define NormalMap_Shader_h

#include "SDL.h"
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#include <string>

#include <stdio.h>

static bool ReadShader(char const * path, char** outData)
{
    FILE *pFile;
    pFile = fopen(path, "rb");
    if (pFile == NULL)
    {
        printf("[ERROR] Can not open file %s", path);
        return false;
    }
    
    size_t size;
    fseek(pFile, 0, SEEK_END);
    size = ftell(pFile);
    rewind(pFile);
    
    *outData = (char*)malloc(sizeof(char)*(size+1));
    memset(*outData, 0, size + 1);
    size_t res_size = fread(*outData, 1, size, pFile);
    if (res_size != size)
    {
        printf("[ERROR] Can not open file %s", path);
        return false;
    }
    
    return true;
}

static GLuint LoadShader(GLenum type, const char* shaderSrc)
{
    GLuint shader;
    GLint compiled = 0;
    
    shader = glCreateShader(type);
    
    if (shader == 0)
    {
        return 0;
    }
    
    glShaderSource(shader, 1, &shaderSrc, NULL);
    
    glCompileShader(shader);
    
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    
    if (!compiled)
    {
        GLint infoLen = 0;
        
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        
        if (infoLen > 1)
        {
            char* infoLog = new char[infoLen];
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            SDL_Log("Error compiling shader: \n%s\n", infoLog);
            delete[] infoLog;
        }
        
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}



static GLuint CompileShader(const std::string & vshader_path, const std::string & fshader_path)
{
    char *vShaderStr;
    ReadShader(vshader_path.c_str(), &vShaderStr);
    
    char * fShaderStr;
    ReadShader(fshader_path.c_str(), &fShaderStr);
    
    GLuint vertexShader;
    GLuint fragmentShader;
    GLint linked;
    
    vertexShader = LoadShader(GL_VERTEX_SHADER, vShaderStr);
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fShaderStr);
    
    GLuint programObject = glCreateProgram();
    if (programObject == 0)
    {
        return 0;
    }
    
    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);
    glLinkProgram(programObject);
    
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
    
    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
        
        if (infoLen > 1)
        {
            char* infoLog = new char[infoLen];
            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            SDL_Log("Error linking program:\n%s\n", infoLog);
            delete [] infoLog;
        }
        glDeleteProgram(programObject);
        return 0;
    }
    
    return programObject;
}


#endif
