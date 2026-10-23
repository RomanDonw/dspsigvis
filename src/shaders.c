#include "shaders.h"

const char *vertexshadersource =
    "#version 430 core\n"

    "layout (location = 0) in vec2 pos;"

    "out vec2 scrcoord;"
    
    "void main(void)"
    "{"
        "scrcoord = vec2((1 + pos.x) * 0.5, pos.y);"
        "gl_Position = vec4(pos, 0, 1);"
    "}"
;

const char *fragmentshadersource =
    "#version 430 core\n"

    "layout (std430, binding = 0) readonly buffer insamples"
    "{"
        "float samples[];"
    "};"
    "uniform uint samplescount;"

    "in vec2 scrcoord;"
    "out vec4 FragmentColor;"
    
    "void main(void)"
    "{"
        "uint idx = clamp(uint(floor(scrcoord.x * samplescount)), 0, samplescount - 1);"
        "float v = samples[idx];"
        
        "if (scrcoord.y < 0) FragmentColor = scrcoord.y + 0.001 >= v ? vec4(1) : vec4(0, 0, 0, 1);"
        "else if (scrcoord.y > 0) FragmentColor = scrcoord.y - 0.001 <= v ? vec4(1) : vec4(0, 0, 0, 1);"
        "else FragmentColor = vec4(1);"
    "}"
;