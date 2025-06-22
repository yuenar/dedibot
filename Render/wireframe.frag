#version 330 core
#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform vec4 color;

out vec4 fragColor;

void main()
{
    fragColor = color;
}

