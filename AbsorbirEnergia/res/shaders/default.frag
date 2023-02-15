#version 330 core
out vec4 FragColor;

in vec2 pass_uvCoordinates;

uniform sampler2D ourTexture;

void main()
{
    FragColor = texture2D(ourTexture, pass_uvCoordinates);
}