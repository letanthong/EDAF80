#version 410

uniform vec3 light_position;

in VS_OUT {
	vec3 vertex;
	vec3 normal;
} fs_in;

out vec4 frag_color;

void main()
{
	vec3 L = normalize(light_position - fs_in.vertex);
	vec3 N = normalize(fs_in.normal);
	vec3 ambient_color = vec3(0.7f, 0.7f, 0.3f); //light yellow
	frag_color = vec4(1.0) * clamp(dot(normalize(fs_in.normal), L), 0.0, 1.0) * vec4(ambient_color, 1.0f);
	
}
