#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 uvCoordinates;

out vec2 pass_uvCoordinates;

uniform mat4 projectionMatrix;
uniform mat4 model;
uniform float spriteCount;
uniform float spriteIndex;

void main()
{
    gl_Position = projectionMatrix * model * vec4(aPos.x, aPos.y, 0.0, 1.0);
    pass_uvCoordinates = uvCoordinates;
    float uvCoordX = (1.0 / spriteCount) * (spriteIndex + uvCoordinates.x);
    pass_uvCoordinates.x = uvCoordX;
}
