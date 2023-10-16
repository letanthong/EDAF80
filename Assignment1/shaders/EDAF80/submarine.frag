#version 410

uniform sampler2D submarine_diff;
uniform sampler2D submarine_rough;
uniform sampler2D submarine_normal;

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
vec3 submarine_text_normal;

void main()
{
	vec3 submarine_diff_color;
	vec3 submarine_rough_color;
	vec3 submarine_normal_color;
	vec3 N_text; //Texture 1 normal vector
	vec3 L; //Light vector
	vec3 V; //Viewer vector

	//Body texture
	submarine_text_normal = texture(submarine_normal, fs_in.texcoord).rgb;
	N_text = normalize(fs_in.TBN * submarine_text_normal);
	L = normalize(light_position - fs_in.vertex);
	V = normalize(camera_position - fs_in.vertex);

	submarine_diff_color = texture(submarine_diff, fs_in.texcoord).rgb;
	submarine_rough_color = texture(submarine_rough, fs_in.texcoord).rgb;

	vec3 diffuse = submarine_diff_color  * max( dot(N_text, L), 0.0);
	vec3 specular = submarine_rough_color * pow( max( dot(reflect(-L,N_text), V) , 0.0 ), shininess ); 

	frag_color.xyz = diffuse + specular;
	frag_color.w = 1.0;

}
