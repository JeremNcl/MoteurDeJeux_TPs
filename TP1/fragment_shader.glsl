#version 330 core

// Input data
in vec2 vUV;

// Ouput data
out vec3 color;

// Uniforms
uniform sampler2D diffuseTexture;

void main(){

        color = texture(diffuseTexture, vUV);
}
