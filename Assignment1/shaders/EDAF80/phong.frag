#version 410

uniform vec3 light_position;
uniform vec3 camera_position;

uniform vec3 ambient_colour;
uniform vec3 diffuse_colour;
uniform vec3 specular_colour;
uniform float shininess_value;

uniform sampler2D diffuse_texture;
uniform sampler2D specular_texture;
uniform sampler2D normal_texture;

uniform bool use_normal_mapping;



in VS_OUT {
	vec3 vertex;
	vec3 normal;
	vec2 text_coord;
	mat3 TBN;
} fs_in;

out vec4 frag_color;

void main()
{
vec3 ka; //Material ambient
vec3 kd; //Material diffuse
vec3 ks; //Material specular
vec3 normal;
vec3 N;
	if(use_normal_mapping == true)
	{
		normal = texture(normal_texture, fs_in.text_coord).rgb;
		normal = normal*2.0 - 1.0;
		N = normalize(fs_in.TBN * normal);
	}
	else
	{
		N = normalize(fs_in.normal); //Normal vector
	}

	kd = texture(diffuse_texture, fs_in.text_coord).rgb;
	ks = texture(specular_texture, fs_in.text_coord).rgb;

	vec3 L = normalize(light_position - fs_in.vertex); //Light vector
	vec3 V = normalize(camera_position - fs_in.vertex); //Viewer vector

	//vec3 ambient = ka * ambient_colour;
	vec3 diffuse = kd  * max( dot(N, L), 0.0);
	vec3 specular = ks * pow( max( dot(reflect(-L,N), V) , 0.0 ), shininess_value ); //where reflect(-L,N) is the ligh reflect vector

	frag_color.xyz = ambient_colour + diffuse + specular;
	frag_color.w = 1.0;
}
