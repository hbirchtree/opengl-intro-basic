#include <stdio.h>
#include <SDL2/SDL.h>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "tga_read.h"

static const char* shader_ver_string = {
    "#version 330 core\n"
};

static const char* shader_vertex_string = {
    "layout(location = 0) in vec3 pos;\n"

    "uniform mat4 mat_f;\n"

    "flat out int instance_id;\n"
    "out vec3 col;\n"

    "void main(void) {\n"
    "    instance_id = gl_InstanceID;\n"
    "    col = pos;\n"
    "    gl_Position = mat_f * vec4(pos,1.);\n"
    "}\n"
};

static const char* shader_fragment_string = {
    "out vec4 g_Color;\n"
    "flat in int instance_id;\n"
    "in vec3 col;\n"
    "void main(void){\n"
    "    g_Color = vec4(col,1.);\n"
    "}\n"
};

static const GLfloat vertex_data_store[3*8] = {
     1.000000,-1.000000,-1.000000,
     1.000000,-1.000000, 1.000000,
    -1.000000,-1.000000, 1.000000,
    -1.000000,-1.000000,-1.000000,
     1.000000, 1.000000,-1.000000,
     1.000000, 1.000000, 1.000000,
    -1.000000, 1.000000, 1.000000,
    -1.000000, 1.000000,-1.000000,
};

static const GLchar index_data_store[12*3] = {
    2,3,4,
    8,7,6,
    5,6,2,
    6,7,3,
    3,7,8,
    1,4,8,
    1,2,4,
    5,8,6,
    1,5,2,
    2,6,3,
    4,3,8,
    5,1,8,
};

void gl_error(void)
{
    int code = 0;
    if((code = glGetError()) != 0)
    {
        fprintf(stdout,"GL error code: %i\n",code);
        fflush(stdout);
    }
}

int main(void)
{
    int closing = 0;
    SDL_Event ev;

    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS);

    SDL_Window* win = SDL_CreateWindow("Hello GL3.3",0,0,800,600,SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL);

    if(!win)
    {
        printf("Failed to init SDL window: %s\n",SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL,SDL_TRUE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,SDL_TRUE);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,8);

    SDL_GLContext glctxt = SDL_GL_CreateContext(win);

    if(!glctxt)
    {
        printf("Failed to obtain GL context from window: %s\n",SDL_GetError());
        return 1;
    }

    SDL_GL_MakeCurrent(win,glctxt);

    gladLoadGL();

    {
        tga_data* tex_data = get_texture_data("texture.tga");

        if(!tex_data || tex_data->header->typecode != 2)
            return 1;

        GLuint program;
        GLuint shaders[2];
        GLuint buffers[2];
        GLuint arrays[1];
        GLuint textures[1];

        const char* shader_vertex_arr[] = {
            shader_ver_string,
            shader_vertex_string
        };

        const char* shader_fragment_arr[] = {
            shader_ver_string,
            shader_fragment_string
        };

        glGenBuffers(2,buffers);
        glGenVertexArrays(1,arrays);
        glGenTextures(1,textures);

        glBindVertexArray(arrays[0]);
        glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,buffers[1]);

        glBufferData(GL_ARRAY_BUFFER,sizeof(vertex_data_store),vertex_data_store,GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(index_data_store),index_data_store,GL_STATIC_DRAW);
        gl_error();

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*3,(const GLvoid*)0x0);

        glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);

        shaders[0] = glCreateShader(GL_VERTEX_SHADER);
        shaders[1] = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(shaders[0],2,shader_vertex_arr,0x0);
        glShaderSource(shaders[1],2,shader_fragment_arr,0x0);

        glCompileShader(shaders[0]);
        glCompileShader(shaders[1]);
        gl_error();

        program = glCreateProgram();

        glAttachShader(program,shaders[0]);
        glAttachShader(program,shaders[1]);

        glLinkProgram(program);
        gl_error();

        glDetachShader(program,shaders[0]);
        glDetachShader(program,shaders[1]);

        glDeleteShader(shaders[0]);
        glDeleteShader(shaders[1]);

        glBindTexture(GL_TEXTURE_2D,textures[0]);

        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,
                     tex_data->header->width,tex_data->header->height,
                     0,
                     GL_RGB,GL_UNSIGNED_BYTE,tex_data->img_data);

        free_texture_data(tex_data);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);

        glActiveTexture(GL_TEXTURE0 + 0);

        glUseProgram(program);
        gl_error();

        GLuint tex_loc = glGetUniformLocation(program,"tex_s");
        GLuint mat_loc = glGetUniformLocation(program,"mat_f");

        glm::mat4 mat_perspective = glm::perspective(90.f,1.3f,1.0f,10.0f);
        glm::mat4 mat_data[1] = {};

        glClearColor(0.5,0.5,0.5,1.0);

        float scale_num = 0.5;
        float variable_time = 0.0;

        printf("Sizeof index array: %lu\n",sizeof(index_data_store));

        glEnable(GL_DEPTH_TEST);

        while(!closing)
        {
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            variable_time = float(SDL_GetTicks())/1000.0;
            scale_num = float(SDL_GetTicks() % 1000 + 50) / 1000;

            mat_data[0] = mat_perspective * glm::translate(glm::mat4(),glm::vec3(sin(variable_time),cos(variable_time),-3));
            mat_data[0] = glm::scale(mat_data[0],glm::vec3(scale_num));

            glUniform1i(tex_loc,0);
            glUniformMatrix4fv(mat_loc,1,GL_FALSE,glm::value_ptr(mat_data[0]));

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,buffers[1]);

            glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_BYTE,0x0);

            /* Check for events */
            while(SDL_PollEvent(&ev))
            {
                /* Do things with events */
                if(ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_CLOSE)
                    closing = 1;
                else if(ev.type == SDL_KEYDOWN)
                {
                    switch(ev.key.keysym.sym)
                    {
                    case SDLK_ESCAPE:
                        closing = 1;
                        break;
                    default:
                        break;
                    };
                }
            }

            SDL_GL_SwapWindow(win);
        }

        glDeleteVertexArrays(1,arrays);
        glDeleteBuffers(1,buffers);
        glDeleteProgram(program);

        gl_error();
    }

    SDL_GL_MakeCurrent(win,0x0);
    SDL_GL_DeleteContext(glctxt);
    SDL_DestroyWindow(win);

    SDL_Quit();

    return 0;
}
