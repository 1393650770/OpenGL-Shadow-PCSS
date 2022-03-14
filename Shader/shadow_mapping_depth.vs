#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;
layout (std140,binding =1) uniform ShadowTypeUBO
{
    int ShadowMapType;
}; 

void main()
{
    gl_Position = ShadowMapType==1 ? model * vec4(aPos, 1.0): lightSpaceMatrix * model * vec4(aPos, 1.0);
}
