#version 330 core
out vec4 FragColor;

uniform float rval;
uniform float gval;
uniform float bval;

void main()
{
    FragColor = vec4(rval, gval, bval, 1.0);
}
