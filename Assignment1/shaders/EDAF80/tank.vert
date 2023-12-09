#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 text_coord;
layout (location = 3) in vec3 tangents;
layout (location = 4) in vec3 binormal;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform float elapsed_time_s;

out VS_OUT {
	vec3 vertex;
	vec3 normal;
	vec2 text_coord;
	vec3 tangents;
	vec3 binormal;
	mat3 TBN;
	float time;
} vs_out;

void main()
{
	vec3 displace_vertex = vertex;
	
	vec3 T = normalize(vs_out.tangents);
	vec3 B = normalize(vs_out.binormal);
	vec3 N = normalize(vs_out.normal);
	vs_out.TBN = mat3(T, B, N);
	vs_out.vertex = vec3(vertex_model_to_world * vec4(displace_vertex, 1.0f));
	vs_out.normal = normal;
	vs_out.text_coord = text_coord.xy;
	vs_out.tangents = tangents;
	vs_out.binormal = binormal;
	vs_out.time = elapsed_time_s;


	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(displace_vertex, 1.0f);
}
