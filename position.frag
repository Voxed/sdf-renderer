#pragma once

#include <string>

std::string fragpos = R"(

#version 440

in vec3 normal;
in vec3 vsnormal;
in vec3 position;


layout(location = 0) out vec4 color;
layout(location = 1) out vec4 normals;

void main()
{
    vec3 li = vec3(-1,1,1);
    vec3 l = normalize(li);

    color = vec4(position, 1.0);
    normals = vec4(normal, 1.0);
    //color = vec4(vec3(normal), 1.0);
}

)";