#pragma once

#include <string>

std::string vertsource = R"(

#version 440

layout(location = 0) in vec3 POSITION;

out vec2 texcoord;

void main()
{
    gl_Position = vec4(POSITION, 1.0);
    texcoord = (POSITION.xy + 1.0)/2.0;
}

)";