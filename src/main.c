#include <dspmodule.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>

#include "shaders.h"

const unsigned short dspmodule_requiredAPIversion = 1;

static GLFWwindow *win;
#define INITWIDTH 800
#define INITHEIGHT 600

static void *inleftport, *inrightport;
static GLuint sampbuffs[2];
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

static bool paused = false;
static void onkey(GLFWwindow *window, int key, int scancode, int action, int mods)
{ if (action == GLFW_PRESS && key == GLFW_KEY_SPACE) paused = !paused; }

unsigned short dspmodule_startup(const DSPLoaderAPI *lapi, int argc, char * const argv[], const char **sysname, const char **dispname)
{
    float fgleftcolor[3] = {1, 0, 1};
    float fgrightcolor[3] = {0, 1, 0};
    float fgalpha = 1;
    float bgcolor[4] = {0, 0, 0, 1};
    {
        int p;
        while ((p = getopt(argc, argv, "b:l:r:a:")) != -1)
        {
            switch (p)
            {
                case 'b':
                    if (sscanf(optarg, "%f,%f,%f,%f", bgcolor, bgcolor + 1, bgcolor + 2, bgcolor + 3) < 4)
                    { puts("error parsing option -b"); return 1; }
                    bgcolor[0] = clampf(bgcolor[0], 0, 1);
                    bgcolor[1] = clampf(bgcolor[1], 0, 1);
                    bgcolor[2] = clampf(bgcolor[2], 0, 1);
                    bgcolor[3] = clampf(bgcolor[3], 0, 1);
                    break;

                case 'l':
                    if (sscanf(optarg, "%f,%f,%f", fgleftcolor, fgleftcolor + 1, fgleftcolor + 2) < 3)
                    { puts("error parsing option -l"); return 1; }
                    fgleftcolor[0] = clampf(fgleftcolor[0], 0, 1);
                    fgleftcolor[1] = clampf(fgleftcolor[1], 0, 1);
                    fgleftcolor[2] = clampf(fgleftcolor[2], 0, 1);
                    break;

                case 'r':
                    if (sscanf(optarg, "%f,%f,%f", fgrightcolor, fgrightcolor + 1, fgrightcolor + 2) < 3)
                    { puts("error parsing option -r"); return 1; }
                    fgrightcolor[0] = clampf(fgrightcolor[0], 0, 1);
                    fgrightcolor[1] = clampf(fgrightcolor[1], 0, 1);
                    fgrightcolor[2] = clampf(fgrightcolor[2], 0, 1);
                    break;

                case 'a':
                    if (sscanf(optarg, "%f", &fgalpha) < 1) { puts("error parsing option -a"); return 1; }
                    fgalpha = clampf(fgalpha, 0, 1);
                    break;
            }
        }
    }

    if (!glfwInit()) { puts("failed to initialize GLFW"); return 1; }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    if (!(win = glfwCreateWindow(INITWIDTH, INITHEIGHT, "Signal visualizer", NULL, NULL)))
    { puts("failed creating GLFW window"); goto errorquit_afterinitglfw; }
    glfwSetKeyCallback(win, onkey);
    glfwMakeContextCurrent(win);
    
    if (!gladLoadGL((void *)glfwGetProcAddress))
    { puts("failed loading OpenGL functions through GLAD"); goto errorquit_afterinitglfw; }
    onresize(win, INITWIDTH, INITHEIGHT);
    glfwSetFramebufferSizeCallback(win, onresize);

    // ================================

    if (!(inleftport = lapi->addport("left", NULL, DSPPortDirection_Input, 0)))
    { puts("failed addding left channel input port"); goto errorquit_afterinitglfw; }
    if (!(inrightport = lapi->addport("right", NULL, DSPPortDirection_Input, 0)))
    { puts("failed addding right channel input port"); goto errorquit_afterinitglfw; }

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
        glUniform4fv(glGetUniformLocation(prog, "bgcolor"), 1, bgcolor);
        glUniform3fv(glGetUniformLocation(prog, "fgleftcolor"), 1, fgleftcolor);
        glUniform3fv(glGetUniformLocation(prog, "fgrightcolor"), 1, fgrightcolor);
        glUniform1f(glGetUniformLocation(prog, "fgalpha"), fgalpha);
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

    glGenBuffers(2, sampbuffs);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sampbuffs[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sampbuffs[1]);

    // ================================

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // ================================

    printf("fgleftcolor: [%f, %f, %f]\nfgrightcolor: [%f, %f, %f]\nfgalpha: %f\nbgcolor: [%f, %f, %f, %f]\n",
        fgleftcolor[0], fgleftcolor[1], fgleftcolor[2], fgrightcolor[0], fgrightcolor[1], fgrightcolor[2], fgalpha,
        bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3]);

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
    if (paused) goto skipupdate;

    const float *inleft = lapi->getportbuffer(inleftport, duration);
    if (!inleft) goto skiprender;
    const float *inright = lapi->getportbuffer(inrightport, duration);
    if (!inright) goto skiprender;
    
    const size_t insize = duration * sizeof(float);
    static size_t currsize = 0;
    if (currsize != insize)
    {
        glNamedBufferData(sampbuffs[0], insize, inleft, GL_DYNAMIC_READ);
        glNamedBufferData(sampbuffs[1], insize, inright, GL_DYNAMIC_READ);
        glUniform1ui(u_samplescount, duration);
        currsize = insize;
    }
    else
    {
        glNamedBufferSubData(sampbuffs[0], 0, insize, inleft);
        glNamedBufferSubData(sampbuffs[1], 0, insize, inright);
    }

    skipupdate:
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, NULL);
    glfwSwapBuffers(win);
    skiprender:
    glfwPollEvents();

    return 0;
}

void dspmodule_cleanup(void) { glfwTerminate(); }