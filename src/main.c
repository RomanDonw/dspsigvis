#include <dspmodule.h>

#include <stdio.h>

#include <glad/gl.h>
#include "main.h"

const unsigned short dspmodule_requiredAPIversion = 1;

GLFWwindow *win;
#define INITWIDTH 800
#define INITHEIGHT 600

static void *inport;

unsigned short dspmodule_startup(const DSPLoaderAPI *lapi, int argc, char * const argv[], const char **sysname, const char **dispname)
{
    if (!glfwInit()) { puts("failed to initialize GLFW"); return 1; }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    if (!(win = glfwCreateWindow(INITWIDTH, INITHEIGHT, "Signal visualizer", NULL, NULL)))
    { puts("failed creating GLFW window"); goto errorquit_afterinitglfw; }
    glfwMakeContextCurrent(win);

    if (!gladLoadGL((void *)glfwGetProcAddress))
    { puts("failed loading OpenGL functions through GLAD"); goto errorquit_afterinitglfw; }

    // ================================

    if (!(inport = lapi->addport("in", NULL, DSPPortDirection_Input, 0)))
    { puts("failed addding single input port"); goto errorquit_afterinitglfw; }

    // ================================

    glClearColor(0.5, 0, 0.5, 1);

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

    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(win);
    glfwPollEvents();

    return 0;
}

void dspmodule_cleanup(void)
{
    glfwTerminate();
}
