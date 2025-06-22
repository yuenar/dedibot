#version 330 core
#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

struct DirectionalLight
{
    vec4 Ambient;
    vec4 Diffuse;
    vec4 Specular;
    vec3 Direction;
};

struct Material
{
    vec4 Ambient;
    vec4 Diffuse;
    vec4 Specular; // w = SpecPower
    vec4 Reflect;
};

uniform DirectionalLight light;
uniform Material mat;

uniform vec3 eyePos;

in vec3 v_position;
in float v_clipDist;

out vec4 fragColor;

void main()
{
    if (v_clipDist < 0.0) discard;

    if(gl_FrontFacing) {
        vec4 ambient = vec4(0.0, 0.0, 0.0, 0.0);
        vec4 diffuse = vec4(0.0, 0.0, 0.0, 0.0);
        vec4 spec    = vec4(0.0, 0.0, 0.0, 0.0);

        vec3 toEye = normalize(eyePos - v_position);
        vec3 lightVec = -light.Direction;

        vec3 normal = normalize(cross(dFdx(v_position), dFdy(v_position)));
        ambient = mat.Ambient * light.Ambient;

        float diffuseFactor = dot(lightVec, normal);

        if(diffuseFactor > 0.0) {
            vec3 v = reflect(-lightVec, normal);
            float specFactor = pow(max(dot(v, toEye), 0.0), mat.Specular.w);

            diffuse = diffuseFactor * mat.Diffuse * light.Diffuse;
            spec = specFactor * mat.Specular * light.Specular;
        }

        vec4 litColor = ambient + diffuse + spec;

        fragColor = litColor;
    } else {
        fragColor = vec4(0.0,0.0,0.0,1.0);
    }
}

