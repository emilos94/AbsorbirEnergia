#version 330 core
layout (location = 0) in vec2 aPos;

out vec4 pass_color;

uniform mat4 projection_matrix;
uniform mat4 model;
uniform vec3 color;

void main()
{
    gl_Position = projection_matrix * model * vec4(aPos.x, aPos.y, 0.0, 1.0);
    pass_color = vec4(color, 1.0);
}
