#version 330 core

layout (location=0) in vec2 position;
layout (location=1) in vec2 uv_coordinate;
layout (location=2) in vec3 text_color;

out vec2 pass_uv_coordinates;
out vec3 pass_text_color;

uniform mat4 projection_matrix;

void main() {
	pass_uv_coordinates = uv_coordinate;
	pass_text_color = text_color;

	gl_Position = projection_matrix * vec4(position.xy, 0.1, 1.0);
}
