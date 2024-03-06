#version 410 core

in vec2 fTexCoords;

out vec4 fColor;

// textures
uniform sampler2D diffuseTexture;

void main() 
{

    vec3 result = vec3(texture(diffuseTexture, fTexCoords));

    fColor = vec4(min(result, 1.0), 1.0f);
}