#pragma once

#include <string>

std::string rastfragsource = R"(

#version 440

in vec3 normal;
in vec3 vsnormal;

layout(location = 0) out vec4 color;

void main()
{
    vec3 li = vec3(-1,1,1);
    vec3 l = normalize(li);

    color = vec4(vec3(dot(normal, l)), 1.0);
    //color = vec4(vec3(normal), 1.0);
}

)";