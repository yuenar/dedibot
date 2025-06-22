#version 330 core
#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D u_texture;

in vec2 v_texcoord;

out vec4 fragColor;

void main()
{
    fragColor = texture(u_texture, v_texcoord);
}

