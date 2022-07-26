#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    //FragColor = vec4(texture(ourTexture, TexCoord).r,1,1,1);
    FragColor = vec4(texture(ourTexture, TexCoord).rgb,1);
}
