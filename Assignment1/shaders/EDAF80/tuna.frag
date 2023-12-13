#version 410

uniform sampler2D tuna_body_diff;
uniform sampler2D tuna_body_rough;
uniform sampler2D tuna_body_normal;

uniform mat4 normal_model_to_world;

uniform vec3 light_position;
uniform vec3 light_position_2;
uniform vec3 camera_position;

uniform vec3 ambient_colour;
uniform vec3 diffuse_colour;
uniform vec3 specular_colour;
uniform float shininess;


in VS_OUT {
	vec3 vertex;
	vec2 texcoord;
	vec3 normal;
	mat3 TBN;
} fs_in;

out vec4 frag_color;
vec3 body_text_normal;

void main()
{
	vec3 tuna_body_diff_color;
	vec3 tuna_body_rough_color;
	vec3 tuna_body_normal_color;
	vec4 frag_color_body;
	vec4 frag_color_eyes;
	vec3 N_body; //Texture 1 normal vector
	vec3 N_eyes; //Texture 2 normal vector
	vec3 L; //Light vector
	vec3 L2; //light vector 2
	vec3 V; //Viewer vector

	//Body texture
	body_text_normal = texture(tuna_body_normal, fs_in.texcoord).rgb;
	body_text_normal = body_text_normal*2.0 -1.0;
	N_body = normalize(fs_in.TBN * body_text_normal);
	L = normalize(light_position - fs_in.vertex);
	L2 = normalize(light_position_2 - fs_in.vertex);
	V = normalize(camera_position - fs_in.vertex);

	tuna_body_diff_color = texture(tuna_body_diff, fs_in.texcoord).rgb;
	tuna_body_rough_color = texture(tuna_body_rough, fs_in.texcoord).rgb;

	vec3 diffuse_body = tuna_body_diff_color  * max( dot(N_body, L), 0.0);
	vec3 diffuse_body_2 = tuna_body_diff_color  * max( dot(N_body, L2), 0.0);
	vec3 specular_body = tuna_body_rough_color * pow( max( dot(reflect(-L,N_body), V) , 0.0 ), shininess );
	vec3 specular_body_2 = tuna_body_rough_color * pow( max( dot(reflect(-L2,N_body), V) , 0.0 ), 5.0f );

	frag_color_body.xyz = diffuse_body + specular_body + diffuse_body_2 + specular_body_2;
	frag_color_body.w = 1.0;

	frag_color = frag_color_body;
//	frag_color = vec4(1.0f);
}
