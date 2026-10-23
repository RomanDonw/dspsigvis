#include "shaders.h"

const char *vertexshadersource =
    "#version 430 core\n"

    "layout (location = 0) in vec2 pos;"

    "out vec2 scrcoord;"
    
    "void main(void)"
    "{"
        "scrcoord = vec2(1 + pos.x, 1 - pos.y) * vec2(0.5);"
        "gl_Position = vec4(pos, 1, 1);"
    "}"
;

const char *fragmentshadersource =
    "#version 430 core\n"

    "in vec2 scrcoord;"
    "out vec4 FragmentColor;"
    
    "void main(void)"
    "{"
        "FragmentColor = vec4(scrcoord, 0.5, 1);"
    "}"
;