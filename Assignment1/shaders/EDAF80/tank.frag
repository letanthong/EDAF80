#version 410

uniform vec3 light_position;
uniform vec3 camera_position;

vec3 N;

in VS_OUT {
	vec3 vertex;
	vec3 normal;
	mat3 TBN;
} fs_in;

out vec4 frag_color;

void main()
{
	vec4 color_deep = vec4(0.0f, 0.0f, 0.1f, 0.5f);
	vec4 color_shallow = vec4(0.0f, 0.5f, 0.5f, 0.7f);
 
	N = normalize(fs_in.TBN * fs_in.normal);

	vec3 L = normalize(light_position - fs_in.vertex); //Light vector
	vec3 V = normalize(camera_position - fs_in.vertex); //Viewer vector
	float facing = 1.0f - max(dot(V, N), 0.0f);
	vec4 water_color = color_deep;

	vec3 R = reflect(-L, N);

	float ambient = 0.1;
	float diffuse = max(dot(N, L), 0.0);
	float specular = pow(max(dot(R, V), 0.0), 32.0); // Adjust the shininess factor as needed

	vec4 diffuse_color = water_color * diffuse;
	vec4 specular_color = vec4(1.0, 1.0, 1.0, 0.5) * specular; // Adjust the specular color as needed

	frag_color = water_color * ambient + diffuse_color + specular_color;

	//frag_color = water_color;//+ refraction_color * (1 - fresnel);
	//frag_color = vec4(0.2, 0.2, 0.2, 0.3);
//	if (frag_color.a == 0.0)
//		discard;
}
