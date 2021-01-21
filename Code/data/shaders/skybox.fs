varying vec3 v_position;
varying vec3 v_normal;

uniform vec3 u_camera_position;
uniform vec4 u_color;
uniform samplerCube u_texture;

varying vec3 v_world_position;

vec4 gamma(vec4 c){

	return pow(c, vec4(1.0/2.2));
}

vec4 toneMap(vec4 color)
{
    return color / (color + vec4(1.0));
}

void main()
{	
	vec3 dir = normalize(v_world_position - u_camera_position);
	vec4 color = textureCube(u_texture, dir);
	color = toneMap(color); //diapositiva 39
	gamma(color);
	gl_FragColor = color;

}