#version 330 core

in vec2 pass_uv_coordinates;
in vec3 pass_text_color;

out vec4 FragColor;

uniform sampler2D font_atlas;

const float width = 0.45;
const float edge = 0.1;

void main() {
	float distance = texture(font_atlas, pass_uv_coordinates).a;
	float alpha = smoothstep(width, width + edge, distance);
	FragColor = vec4(vec3(1.0), alpha);
}
