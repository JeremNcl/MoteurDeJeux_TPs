#version 330 core

// Input data
in vec2 vUV;
in float vHeight;

// Ouput data
out vec3 color;

// Uniforms - Textures pour différentes hauteurs
uniform sampler2D texture_low;      // Texture basse (herbe, sable)
uniform sampler2D texture_mid;      // Texture moyenne (roche, terre)
uniform sampler2D texture_high;     // Texture haute (neige, roche)

// Paramètres de hauteur pour le blending
uniform float height_low;    // Hauteur max pour texture_low
uniform float height_mid;    // Hauteur max pour texture_mid
uniform float blend_range;   // Zone de transition entre textures

void main(){
    vec3 tex_low = texture(texture_low, vUV).rgb;
    vec3 tex_mid = texture(texture_mid, vUV).rgb;
    vec3 tex_high = texture(texture_high, vUV).rgb;
    
    // Calculer les facteurs de mélange
    float factor_low = smoothstep(height_low - blend_range, height_low + blend_range, vHeight);
    float factor_mid = smoothstep(height_mid - blend_range, height_mid + blend_range, vHeight);
    
    // Mélanger les textures en fonction de la hauteur
    color = mix(tex_low, tex_mid, factor_low);
    color = mix(color, tex_high, factor_mid);
}
