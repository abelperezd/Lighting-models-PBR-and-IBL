#define pi 3.1415926535897932384626433832795
#define GAMMA 2.2

//this var comes from the vertex shader
//they are baricentric interpolated by pixel according to the distance to every vertex
varying vec3 v_position;
varying vec3 v_normal;
varying vec3 v_world_position;
varying vec2 v_uv;

//camera
uniform vec3 u_camera_position;
uniform float u_time;

//light
uniform vec3 light_color;
uniform float light_intensity; 
uniform vec3 light_pos;

//textures
uniform sampler2D u_texture_albedo;
uniform sampler2D u_texture_normal;
uniform sampler2D u_texture_metalness;
uniform sampler2D u_texture_roughness;
uniform sampler2D u_texture_emissive;

// Levels of the HDR Environment to simulate roughness material (IBL)
uniform samplerCube u_texture;
uniform samplerCube u_texture_prem_0;
uniform samplerCube u_texture_prem_1;
uniform samplerCube u_texture_prem_2;
uniform samplerCube u_texture_prem_3;
uniform samplerCube u_texture_prem_4;

//LUT Texture
uniform sampler2D u_LUT_texture;

//Selectors
uniform float shadSelect;
uniform bool objectSelect;



//------------------------------FUNCTIONS------------------------------
	//----https://learnopengl.com/PBR/Theory-------
	vec3 fresnelSchlick(float HdotV, vec3 F0)
	{
    		return F0 + (vec3(1.0) - F0) * pow(1.0 - HdotV, 5.0);
	}

	float computeG(float roughness, float NdotL, float NdotV)
	{
		float k = pow(roughness + 1.0 , 2.0)/8.0;
		float G1 = NdotL/(NdotL*(1.0 - k) + k);
		float G2 = NdotV/(NdotV*(1.0 - k) + k);
		return G1*G2;
	}

	float computeD(float roughness, float NdotH)
	{
		float alpha = pow(roughness,2.0);
		float alpha_2 = pow(alpha,2.0);

		float NdotH2 = pow(NdotH,2.0);
		float alpha2_1 = alpha_2 - 1.0;
		float denompart_2 = pow(NdotH2*alpha2_1 + 1.0,2.0);
		float denom = denompart_2 * pi;
		return alpha_2/denom;
	}

	
	//-----------------------GIVEN FUNCTIONS-----------------------
	vec3 degamma(vec3 c){return pow(c, vec3(GAMMA));}
	vec3 gamma(vec3 c){return pow(c, vec3(1.0/GAMMA));}

	vec3 toneMap(vec3 color)
	{
    		return color / (color + vec3(1.0));
	}

	mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
	{
		// get edge vectors of the pixel triangle
		vec3 dp1 = dFdx( p );
		vec3 dp2 = dFdy( p );
		vec2 duv1 = dFdx( uv );
		vec2 duv2 = dFdy( uv );

		// solve the linear system
		vec3 dp2perp = cross( dp2, N );
		vec3 dp1perp = cross( N, dp1 );
		vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
		vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

		// construct a scale-invariant frame
		float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
		return mat3( T * invmax, B * invmax, N );
	}

	vec3 perturbNormal( vec3 N, vec3 V, vec2 texcoord, vec3 normal_pixel ){
		#ifdef USE_POINTS
		return N;
		#endif

		// assume N, the interpolated vertex normal and
		// V, the view vector (vertex to eye)
		//vec3 normal_pixel = texture2D(normalmap, texcoord ).xyz;
		normal_pixel = normal_pixel * 255./127. - 128./127.;
		mat3 TBN = cotangent_frame(N, V, texcoord);
		return normalize(TBN * normal_pixel);
	}
	
	vec3 getReflectionColor(vec3 r, float roughness)
	{
		float lod = roughness * 5.0;

		vec4 color;

		if(lod < 1.0) color = mix( textureCube(u_texture, r), textureCube(u_texture_prem_0, r), lod );
		else if(lod < 2.0) color = mix( textureCube(u_texture_prem_0, r), textureCube(u_texture_prem_1, r), lod - 1.0 );
		else if(lod < 3.0) color = mix( textureCube(u_texture_prem_1, r), textureCube(u_texture_prem_2, r), lod - 2.0 );
		else if(lod < 4.0) color = mix( textureCube(u_texture_prem_2, r), textureCube(u_texture_prem_3, r), lod - 3.0 );
		else if(lod < 5.0) color = mix( textureCube(u_texture_prem_3, r), textureCube(u_texture_prem_4, r), lod - 4.0 );
		else color = textureCube(u_texture_prem_4, r);

		// Gamma correction
		//color = pow(color, vec4(1.0/GAMMA));

		return color.rgb;
	}

void main()
{
	vec2 uv = v_uv;
	vec3 albedo = degamma(texture2D( u_texture_albedo, uv ).rgb);
	vec3 normal_text = texture2D( u_texture_normal, uv ).rgb;
	
	vec3 roughness_metalness = vec3(0,0,0);
	float roughness = 0.0;
	float metalness = 0.0;
	vec3 emissive = vec3(0,0,0);
	
	if(objectSelect == false){ //false -> helmet
		roughness_metalness = texture2D( u_texture_roughness, uv ).rgb;
		roughness = roughness_metalness.g;
		metalness = roughness_metalness.b; 
		emissive = degamma(texture2D( u_texture_emissive, uv ).rgb); //https://community.khronos.org/t/glow-in-the-dark-textures/43658
	
	}else if(objectSelect == true){ //true -> sphere or lantern
		roughness =  texture2D( u_texture_roughness, uv ).r;
		metalness =  texture2D( u_texture_metalness, uv ).r;
	}
	
	//Vectors------------------------
	vec3 V = normalize(u_camera_position-v_world_position);
	vec3 N = normalize(v_normal);
	vec3 L = normalize(light_pos - v_world_position);
	vec3 H = normalize(V + L);	
	vec3 R = normalize(reflect(-L, N)); 

	//Normal maps--------------------
	N = normalize(perturbNormal(N, V, uv, normal_text));

	//Precomputed dots---------------
	float NdotV = clamp(dot(N,V), 0.00000000001, 0.99999999999);
	float NdotL = clamp(dot(N,L), 0.00000000001, 0.99999999999);
	float NdotH = clamp(dot(N,H), 0.00000000001, 0.99999999999);
	float HdotV = clamp(dot(H,V), 0.00000000001, 0.99999999999);
	
	//---------------------------------BSDF---------------------------------
		//Diffuse------------------------------
		vec3 c_diff = (1.0-metalness)*albedo;
		vec3 f_diff = c_diff/pi;

		//Specular----------------------------- https://learnopengl.com/PBR/Theory
		vec3 F0 = metalness*albedo + (1.0-metalness)*vec3(0.04); 
		vec3 F = fresnelSchlick(HdotV, F0);
		float G = computeG(roughness, NdotL, NdotV);
		float D = computeD(roughness, NdotH);
		vec3 f_spec = (F*G*D)/(4.0*NdotL*NdotV);
		
		//Specular + Diffuse--------------------
		vec3 f = f_diff + f_spec;

		//Light--------------------------------
		vec3 Li = light_intensity*light_color;

		vec3 BSDF = f*Li*NdotL;

	//---------------------------------IBL----------------------------------
		//Diffuse------------------------------
		vec3 diffuseSample = getReflectionColor(N, 1.0); //roughness diapo 43
		vec3 diffuseColor = c_diff;
		vec3 diffuseIBL =  diffuseSample*diffuseColor;

		//Specular-----------------------------
		vec3 R_reflective = normalize(reflect(-V,N));
		vec3 specularSample = getReflectionColor(R_reflective, roughness);

		vec2 uv_LUT = vec2(NdotV, roughness); //diapositiva 45 
		vec4 brdf2D = texture2D( u_LUT_texture, uv_LUT );
		vec3 specularBRDF = F0*brdf2D.x + brdf2D.y;

		vec3 specularIBL = specularBRDF * specularSample;

		//Specular + Diffuse--------------------
		vec3 IBL = specularIBL + diffuseIBL;

	//Ilumination selection-----------------
	vec3 finalColor = vec3(0,0,0);	
	if(shadSelect == 0.0){
		finalColor = BSDF;
	}
	else if(shadSelect == 1.0){ 
		finalColor = IBL;
	}
	else if(shadSelect == 2.0){
		finalColor = BSDF+IBL;
	}

	finalColor = toneMap (finalColor);

	if(objectSelect == false){ //helmet
		finalColor = gamma(finalColor+emissive);
	}
	else if(objectSelect == true){ //sphere or lantern
		finalColor = gamma(finalColor);
	}

	gl_FragColor = vec4(finalColor, 1.0);

}