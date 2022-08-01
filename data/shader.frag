#version 460 core
precision highp float;
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D Tex;

void main(){
    FragColor = texture(Tex, TexCoord);
}
