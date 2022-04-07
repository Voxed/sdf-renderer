#pragma once

#include <string>

std::string fragsource = R"(

#version 440

layout(location = 0) out vec4 color;
layout(binding = 0) uniform sampler2D tex;

in vec2 texcoord;

void main()
{
    color = texture(tex, texcoord);
}

)";