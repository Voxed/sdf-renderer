#pragma once

#include <string>

std::string shadfragsource = R"(

#version 440

layout(location = 0) out vec4 color;
layout(binding = 0) uniform sampler2D tex;
layout(binding = 2) uniform sampler2D shadowTex;

in vec2 texcoord;

void main()
{
    color = texture(tex, texcoord)*texture(shadowTex, texcoord);
}

)";