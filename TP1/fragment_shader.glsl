#version 330 core

// Input data
in vec2 vUV;

// Ouput data
out vec3 color;

// Uniforms - Textures pour différentes hauteurs
uniform sampler2D textureSampler;


void main(){
    vec3 tex = texture(textureSampler, vUV).rgb;
    
    // Mélanger les textures en fonction de la hauteur
    color = tex;
}
