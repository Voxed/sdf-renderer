#pragma once

#include <string>

std::string rastvertsource = R"(

#version 440

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;

layout(location = 0) uniform mat4 PROJ;
layout(location = 1) uniform mat4 MODEL;
layout(location = 2) uniform mat4 VIEW;

out vec3 normal;
out vec3 vsnormal;
out vec3 position;

void main()
{
    normal = (inverse(transpose(MODEL))*vec4(NORMAL, 1.0)).xyz;
    vsnormal = (inverse(transpose(VIEW*MODEL))*vec4(NORMAL, 1.0)).xyz;
    gl_Position = PROJ*VIEW*MODEL*vec4(POSITION, 1.0);
    position = (MODEL*vec4(POSITION,1.0)).xyz;
}

)";