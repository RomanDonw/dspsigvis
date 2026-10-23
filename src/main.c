#include <dspmodule.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <stdio.h>
#include <stdlib.h>

#include "shaders.h"

const unsigned short dspmodule_requiredAPIversion = 1;

static GLFWwindow *win;
#define INITWIDTH 800
#define INITHEIGHT 600

static void *inport;
static GLuint sampbuff;
static GLint u_samplescount;

static char *getshadercompilelog(GLuint shader)
{
    GLint size;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
    if (size <= 0) return NULL;
    
    char *ret = malloc(size);
    if (!ret) return NULL;
    glGetShaderInfoLog(shader, size, NULL, ret);
    
    return ret;
}

static void onresize(GLFWwindow *window, int width, int height)
{ glViewport(0, 0, width > 1 ? width : 1, height > 1 ? height : 1); }

unsigned short dspmodule_startup(const DSPLoaderAPI *lapi, int argc, char * const argv[], const char **sysname, const char **dispname)
{
    if (!glfwInit()) { puts("failed to initialize GLFW"); return 1; }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    if (!(win = glfwCreateWindow(INITWIDTH, INITHEIGHT, "Signal visualizer", NULL, NULL)))
    { puts("failed creating GLFW window"); goto errorquit_afterinitglfw; }
    glfwSetFramebufferSizeCallback(win, onresize);
    glfwMakeContextCurrent(win);

    if (!gladLoadGL((void *)glfwGetProcAddress))
    { puts("failed loading OpenGL functions through GLAD"); goto errorquit_afterinitglfw; }

    // ================================

    if (!(inport = lapi->addport("in", NULL, DSPPortDirection_Input, 0)))
    { puts("failed addding single input port"); goto errorquit_afterinitglfw; }

    // ================================

    {
        GLint status;
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vertexshadersource, NULL);
        glCompileShader(vs);
        glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
        if (!status)
        {
            char *log = getshadercompilelog(vs);
            if (!log) { puts("failed getting vertex shader compilation log"); goto errorquit_afterinitglfw; }
            
            printf("vertex shader compilation failed:\n%s", log);
            free(log);
            goto errorquit_afterinitglfw;
        }

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fragmentshadersource, NULL);
        glCompileShader(fs);        
        glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
        if (!status)
        {
            char *log = getshadercompilelog(fs);
            if (!log) { puts("failed getting fragment shader compilation log"); goto errorquit_afterinitglfw; }
            
            printf("fragment shader compilation failed:\n%s", log);
            free(log);
            goto errorquit_afterinitglfw;
        }

        GLuint prog = glCreateProgram();
        glAttachShader(prog, vs);
        glAttachShader(prog, fs);
        glLinkProgram(prog);
        glGetProgramiv(prog, GL_LINK_STATUS, &status);
        if (!status) { puts("shader program linking failed"); goto errorquit_afterinitglfw; }
        
        glUseProgram(prog);

        u_samplescount = glGetUniformLocation(prog, "samplescount");
    }

    // ================================

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    static const char vertices[] = 
    {
        -1, -1,
        1, -1,
        -1, 1,
        1, 1
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_BYTE, GL_FALSE, 2, NULL);
    glEnableVertexAttribArray(0);

    static const unsigned char indices[] =
    {
        0, 1, 3,
        3, 2, 0
    };
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // ================================

    glGenBuffers(1, &sampbuff);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sampbuff);

    // ================================

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // ================================

    *sysname = "dspsigvis";
    *dispname = "signal visualizer";
    return 0;

    errorquit_afterinitglfw:
        glfwTerminate();
    return 1;
}

unsigned short dspmodule_process(const DSPLoaderAPI *lapi, unsigned long long position, unsigned long duration, unsigned long rate, unsigned long long nsectime)
{
    glfwMakeContextCurrent(win);
    if (glfwWindowShouldClose(win)) return 1;

    const float *in = lapi->getportbuffer(inport, duration);
    if (!in) goto skiprender;
    const size_t insize = duration * sizeof(float);
    
    static size_t currsize = 0;
    if (currsize != insize)
    {
        glBufferData(GL_SHADER_STORAGE_BUFFER, insize, in, GL_DYNAMIC_READ);
        currsize = insize;
    }
    else glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, insize, in);

    glUniform1ui(u_samplescount, duration);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, NULL);

    glfwSwapBuffers(win);
    skiprender:
    glfwPollEvents();

    return 0;
}

void dspmodule_cleanup(void)
{
    glfwTerminate();
}
