#include <stdio.h>
#include <SDL.h>

#include <glad/glad.h>

#include "tga_read.h"

static const char* shader_ver_string = {
    "#version 330 core\n"
};

static const char* shader_vertex_string = {
    "layout(location = 0) in vec3 pos;\n"
    "layout(location = 1) in vec2 tex;\n"

    "flat out int instance_id;\n"
    "out vec2 tex_c;"

    "void main(void) {\n"
    "    instance_id = gl_InstanceID;\n"
    "    tex_c = tex;\n"
    "    gl_Position = vec4(pos,1.);\n"
    "}\n"
};

static const char* shader_fragment_string = {
    "out vec4 g_Color;\n"

    "uniform sampler2D tex_s;\n"

    "flat in int instance_id;\n"
    "in vec2 tex_c;\n"

    "void main(void){\n"
    "    g_Color = texture(tex_s,tex_c);\n"
    "}\n"
};

static const GLfloat vertex_data_store[5*6] = {
    -1.f, -1.f,  0.f,     0.f,  0.f,
     1.f, -1.f,  0.f,    -1.f,  0.f,
    -1.f,  1.f,  0.f,     0.f, -1.f,

    -1.f,  1.f,  0.f,     0.f, -1.f,
     1.f,  1.f,  0.f,    -1.f, -1.f,
     1.f, -1.f,  0.f,    -1.f,  0.f,
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

typedef struct rgba_t
{
    char r;
    char g;
    char b;
    char a;
} rgba;

#undef main

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

    SDL_GL_SetSwapInterval(0);

    gladLoadGL();

    {
        GLuint program;
        GLuint shaders[2];
        GLuint buffers[3];
        GLuint arrays[1];
        GLuint textures[2];

        const char* shader_vertex_arr[] = {
            shader_ver_string,
            shader_vertex_string
        };

        const char* shader_fragment_arr[] = {
            shader_ver_string,
            shader_fragment_string
        };

        glGenBuffers(3,buffers);
        glGenVertexArrays(1,arrays);
        glGenTextures(2,textures);

        glBindVertexArray(arrays[0]);
        glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);

        glBufferData(GL_ARRAY_BUFFER,sizeof(vertex_data_store),vertex_data_store,GL_STATIC_DRAW);
        gl_error();

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*5,(const GLvoid*)0x0);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*5,(const GLvoid*)(sizeof(GLfloat)*3));

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
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,buffers[1]);

        size_t pixel_buf_size = 512*512*sizeof(rgba);
        rgba* pixel_data = malloc(pixel_buf_size);

        rgba* curr = 0x0;
        int i,j;
        for(i=0;i<512;i++)
            for(j=0;j<512;j++)
            {
                curr = &pixel_data[i*512 + j];
                curr->r = curr->a = 255;
            }

        glBufferData(GL_PIXEL_UNPACK_BUFFER,pixel_buf_size,pixel_data,GL_STATIC_DRAW);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,
                     512,512,
                     0,
                     GL_RGBA,GL_UNSIGNED_BYTE,0x0);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);

        glBindTexture(GL_TEXTURE_2D,textures[1]);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,
                     512,512,
                     0,
                     GL_RGBA,GL_UNSIGNED_BYTE,0x0);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);

        glActiveTexture(GL_TEXTURE0 + 0);

        glUseProgram(program);
        gl_error();

        GLuint tex_loc = glGetUniformLocation(program,"tex_s");

        unsigned int pbo_idx = 0;

        glClearColor(0.5,0.5,0.5,1.0);
        unsigned long frame_timer = SDL_GetTicks() + 1000;
        unsigned long frame_tick = 0;
        while(!closing)
        {
            glClear(GL_COLOR_BUFFER_BIT);

            unsigned char pixel_val = SDL_GetTicks()/10 % 255;

            for(i=0;i<512;i++)
                for(j=0;j<512;j++)
                {
                    pixel_data[i*512 + j].r = pixel_val;
                }

//            glBindBuffer(GL_PIXEL_UNPACK_BUFFER,buffers[1 + pbo_idx]);
//            glBufferData(GL_PIXEL_UNPACK_BUFFER,pixel_buf_size,pixel_data,GL_STATIC_DRAW);

//            glBindTexture(GL_TEXTURE_2D,textures[pbo_idx]);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,
                         512,512,
                         0,
                         GL_RGBA,GL_UNSIGNED_BYTE,pixel_data);

//            pbo_idx++;
//            pbo_idx = pbo_idx % 2;

//            glBindTexture(GL_TEXTURE_2D,textures[pbo_idx]);

            glUniform1i(tex_loc,0);

            glDrawArraysInstanced(GL_TRIANGLES,0,6,1);

            if(SDL_GetTicks() >= frame_timer)
            {
                frame_timer = SDL_GetTicks() + 1000;
                fprintf(stdout,"FPS: %lu\n",frame_tick);
                fflush(stdout);
                frame_tick = 0;
            }
            frame_tick++;

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
