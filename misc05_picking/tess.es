#version 410 core

layout(isolines, equal_spacing, ccw) in;

uniform mat4 VP;

in vec3 WorldPos_ES_in[];

void main()                                                                                     
{
    float t = gl_TessCoord.x;

    vec3 p0 = WorldPos_ES_in[0];
    vec3 p1 = WorldPos_ES_in[1];
    vec3 p2 = WorldPos_ES_in[2];
    vec3 p3 = WorldPos_ES_in[3];
    vec3 p4 = WorldPos_ES_in[4];

    float a0 = pow(1-t, 4);
    float a1 = pow(1-t, 3) * pow(t, 1) * 4;
    float a2 = pow(1-t, 2) * pow(t, 2) * 6;
    float a3 = pow(1-t, 1) * pow(t, 3) * 4;
    float a4 = pow(t, 4);

    vec3 P = p0*a0 + p1*a1 + p2*a2 + p3*a3 + p4*a4;

    gl_Position = VP * vec4(P, 1.0);
}                                                                                               
