$input v_pos, v_view, v_normal, v_uv

#include "bgfx_shader.sh"

uniform vec4 u_matAmbient;
uniform vec4 u_matDiffuse;
uniform vec4 u_matSpecular;
uniform vec4 u_matShininess;

uniform vec4 u_lightDir;
uniform vec4 u_lightAmbient;
uniform vec4 u_lightDiffuse;
uniform vec4 u_lightSpecular;

void main() {
    vec3 norm = normalize(v_normal);
    vec3 view_dir = normalize(v_view - v_pos);
    vec3 light_dir = normalize(-u_lightDir.xyz);

    // diffuse shading
    float diff = max(dot(norm, light_dir), 0.0);

    // specular shading
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), u_matShininess.x);

    // combine results
    vec3 ambient  = u_lightAmbient.rgb * u_matAmbient.rgb;
    vec3 diffuse  = u_lightDiffuse.rgb  * diff * u_matDiffuse.rgb;
    vec3 specular = u_lightSpecular.rgb * (spec * u_matSpecular.rgb);
    vec3 result = vec3(ambient + diffuse + specular);

	gl_FragColor = vec4(result, 1.0);
}
