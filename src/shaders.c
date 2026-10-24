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

    "uniform uint samplescount;"
    "layout (std430, binding = 0) readonly buffer insamplesleft"
    "{"
        "float leftdata[];"
    "};"
    "layout (std430, binding = 1) readonly buffer insamplesright"
    "{"
        "float rightdata[];"
    "};"

    "uniform vec4 bgcolor;"
    "uniform vec3 fgleftcolor;"
    "uniform vec3 fgrightcolor;"
    "uniform float fgalpha;"

    "in vec2 scrcoord;"
    "out vec4 FragmentColor;"
    
    "void main(void)"
    "{"
        "uint lastidx = samplescount - 1;"
        "uint curridx = uint(floor(scrcoord.x * float(lastidx)));"
        "uint nextidx = min(curridx + 1, samplescount - 1);"
        
        "float currx = float(curridx) / float(lastidx);"
        "float nextx = float(nextidx) / float(lastidx);"
        "float centx = mix(currx, nextx, 0.5);"
        
        "float left = mix(leftdata[curridx], leftdata[nextidx], (scrcoord.x - currx) / (nextx - currx));"
        "float right = mix(rightdata[curridx], rightdata[nextidx], (scrcoord.x - currx) / (nextx - currx));"

        "bool haswide = scrcoord.x >= centx - 0.05 && scrcoord.x <= centx + 0.05;"
        "bool hasleft = scrcoord.y >= left - 0.01 && scrcoord.y <= left + 0.01;"
        "bool hasright = scrcoord.y <= right + 0.01 && scrcoord.y >= right - 0.01;"
        
        "if (haswide && (hasleft || hasright))"
            "FragmentColor = vec4((hasleft ? fgleftcolor : vec3(0)) + (hasright ? fgrightcolor : vec3(0)), fgalpha);"
        "else FragmentColor = bgcolor;"
    "}"
;