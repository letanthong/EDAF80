#version 410

uniform sampler2D shark_diff;
uniform sampler2D shark_rough;
uniform sampler2D shark_normal;

uniform vec3 light_position;
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
vec3 shark_text_normal;

void main()
{
	vec3 shark_diff_color;
	vec3 shark_rough_color;
	vec3 shark_normal_color;
	vec3 N_text; //Texture 1 normal vector
	vec3 L; //Light vector
	vec3 V; //Viewer vector

	//Body texture
	shark_text_normal = texture(shark_normal, fs_in.texcoord).rgb;
	N_text = normalize(fs_in.TBN * shark_text_normal);
	L = normalize(light_position - fs_in.vertex);
	V = normalize(camera_position - fs_in.vertex);

	shark_diff_color = texture(shark_diff, fs_in.texcoord).rgb;
	shark_rough_color = texture(shark_rough, fs_in.texcoord).rgb;

	vec3 diffuse = shark_diff_color  * max( dot(N_text, L), 0.0);
	vec3 specular = shark_rough_color * pow( max( dot(reflect(-L,N_text), V) , 0.0 ), shininess ); 

	frag_color.xyz = diffuse + specular;
	frag_color.w = 1.0;

}
