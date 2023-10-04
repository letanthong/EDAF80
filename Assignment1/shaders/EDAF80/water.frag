#version 410

uniform vec3 light_position;
uniform vec3 camera_position;

uniform vec3 ambient_colour;
uniform vec3 diffuse_colour;
uniform vec3 specular_colour;
uniform float shininess_value;

uniform sampler2D water_normal_texture;
uniform bool use_normal_mapping;

vec3 ka; //Material ambient
vec3 kd; //Material diffuse
vec3 ks; //Material specular
vec3 normal;
vec3 N;

in VS_OUT {
	vec3 vertex;
	vec3 normal;
	vec2 text_coord;
	mat3 TBN;
} fs_in;

out vec4 frag_color;

void main()
{
	/*
	vec3 N;
	if(use_normal_mapping == true)
	{
		normal = texture(water_normal_texture, fs_in.text_coord).rgb;
		normal = normal*2.0 - 1.0;
		N = normalize(fs_in.TBN * normal);
	}
	else
	{
		N = normalize(fs_in.normal);
	}
	

	vec3 L = normalize(light_position - fs_in.vertex); //Light vector
	vec3 V = normalize(camera_position - fs_in.vertex); //Viewer vector
	vec3 ambient = ambient_colour;
	vec3 diffuse = diffuse_colour * max( dot(N, L), 0.0);
	vec3 specular = specular_colour * pow( max( dot(reflect(-L,N), V) , 0.0 ), shininess_value ); //where reflect(-L,N) is the ligh reflect vector
	frag_color = vec4(ambient + diffuse + specular, 1.0);
	*/

	vec3 L = normalize(light_position - fs_in.vertex);
	frag_color = vec4(1.0) * clamp(dot(normalize(fs_in.normal), L), 0.0, 1.0);
}
