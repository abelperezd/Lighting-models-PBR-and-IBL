//this var comes from the vertex shader
//they are baricentric interpolated by pixel according to the distance to every vertex
varying vec3 v_position;
varying vec3 v_normal;
varying vec3 v_world_position;


//here create uniforms for all the data we need here

//material
uniform vec3  ka;
uniform vec3  kd;
uniform vec3  ks;
uniform float alpha;

//luz
uniform vec3  ia;
uniform vec3  id;
uniform vec3  is;
uniform vec3 light_pos;

//camara
uniform vec3 eye_pos;
uniform float u_time;

//texture
uniform sampler2D u_texture;
varying vec2 v_uv;

void main()
{
	//-------------------------------
	vec3 v_wNormal_ = normalize(v_normal);

	vec3 L = normalize(light_pos - v_position);
	vec3 R = reflect((-1)* v_wNormal_, L);
	vec3 V = normalize(eye_pos - v_position);
	vec3 Ip = (ka*ia )+ (kd*max(dot(L, v_wNormal_),0)*id) + (ks*max(pow(dot(R, V), alpha), 0)*is);
	
	//--------------------------------
	vec3 color = Ip;
	gl_FragColor = vec4(color, 1.0) * texture2D( u_texture, v_uv );

	//gl_FragColor = vec4( color, 1.0 ); //sin textura
}