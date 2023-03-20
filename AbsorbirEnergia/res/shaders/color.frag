#version 330 core
out vec4 FragColor;

out vec4 pass_color;

void main()
{
    FragColor = pass_color;
}
