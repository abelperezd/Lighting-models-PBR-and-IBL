varying vec3 v_position;
varying vec3 v_normal;
uniform vec3 u_camera_position;
uniform samplerCube u_texture;

void main()
{
	vec3 v_normal_ = normalize(v_normal);
	vec3 V = normalize(v_position-u_camera_position);
	vec3 R = reflect(V, v_normal_);

	gl_FragColor = texture(u_texture,R);
}
