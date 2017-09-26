#version 410 core

layout (location = 0) in vec4 Position_VS_in;
                                                                                                
uniform mat4 M;

out vec3 WorldPos_CS_in;

void main()
{
    // Didn't multiply view matrix, do it later in TES
    WorldPos_CS_in = (M * vec4(Position_VS_in)).xyz;
}
