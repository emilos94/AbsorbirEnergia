#version 330 core
out vec4 FragColor;

in vec2 pass_uvCoordinates;

uniform sampler2D ourTexture;

uniform vec3 tintColor;
uniform float tintStrength;

void main()
{
    FragColor = texture2D(ourTexture, pass_uvCoordinates);   
    float alpha = FragColor.a;
    FragColor = mix(FragColor, vec4(tintColor, alpha), tintStrength);
}