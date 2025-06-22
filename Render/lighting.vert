#version 330 core
#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

uniform mat4 mvp_matrix;
uniform mat4 mv_matrix;
uniform mat4 trans_matrix;

uniform vec4 clipPlane;

out vec3 v_position;
out float v_clipDist;

void main()
{
    gl_Position = mvp_matrix * vec4(a_position,1.0);

    v_position = vec3(mv_matrix * vec4(a_position, 1.0));
    v_clipDist = dot(vec3(trans_matrix * vec4(a_position, 1.0)), vec3(clipPlane)) + clipPlane.w;
}
