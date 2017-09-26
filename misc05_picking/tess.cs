#version 410 core

// define the number of CPs in the output patch
layout (vertices = 5) out;

// attributes of the input CPs
in vec3 WorldPos_CS_in[];

// attributes of the output CPs
out vec3 WorldPos_ES_in[];

void main()
{
    // Set the control points of the output patch
    WorldPos_ES_in[gl_InvocationID] = WorldPos_CS_in[gl_InvocationID];

    // Using isoline in TES, so only outer 0 & 1 will be used
    gl_TessLevelOuter[0] = 20;  // Vertical
    gl_TessLevelOuter[1] = 20;  // Horizontal
}                                                                                               
