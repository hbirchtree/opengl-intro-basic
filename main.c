#include <stdio.h>
#include <SDL2/SDL.h>

#include <glad/glad.h>

static const char* shader_ver_string = {
    "#version 330 core\n"
};

static const char* shader_vertex_string = {
    "layout(location = 0) in vec3 pos;\n"

    "void main(void) {\n"
    "    gl_Position = vec4(pos,1.);\n"
    "}\n"
};

static const char* shader_fragment_string = {
    "out vec4 g_Color;\n"

    "void main(void){\n"
    "    g_Color = vec4(1.,0.,0.,1.);\n"
    "}\n"
};

static const GLfloat vertex_data_store[3*6] = {
    -1.f, -1.f,  0.f,
     1.f, -1.f,  0.f,
    -1.f,  1.f,  0.f,

    -1.f,  1.f,  0.f,
     1.f,  1.f,  0.f,
     1.f, -1.f,  0.f,
};

void gl_error()
{
    int code = 0;
    if((code = glGetError()) != 0)
        printf("GL error code: %i\n",code);
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
        GLuint program;
        GLuint shaders[2];
        GLuint buffers[1];
        GLuint arrays[1];

        const char* shader_vertex_arr[] = {
            shader_ver_string,
            shader_vertex_string
        };

        const char* shader_fragment_arr[] = {
            shader_ver_string,
            shader_fragment_string
        };

        glGenBuffers(1,buffers);
        glGenVertexArrays(1,arrays);

        glBindVertexArray(arrays[0]);
        glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);

        glBufferData(GL_ARRAY_BUFFER,sizeof(vertex_data_store),vertex_data_store,GL_STATIC_DRAW);
        gl_error();

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(GLfloat)*3,(const GLvoid*)0x0);

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

        glUseProgram(program);
        gl_error();

        glDeleteShader(shaders[0]);
        glDeleteShader(shaders[1]);

        glClearColor(0.5,0.5,0.5,1.0);
        while(!closing)
        {
            glClear(GL_COLOR_BUFFER_BIT);

            glDrawArrays(GL_TRIANGLES,0,6);

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
    }

    SDL_GL_MakeCurrent(win,0x0);
    SDL_GL_DeleteContext(glctxt);
    SDL_DestroyWindow(win);

    SDL_Quit();

    return 0;
}

