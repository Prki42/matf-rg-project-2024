//#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}

//#shader fragment
#version 330 core

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform float opacity;

void main()
{
    if (opacity < 1.0)
        discard;
}
