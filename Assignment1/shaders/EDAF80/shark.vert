#version 410

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 texcoord;
layout (location = 3) in vec3 tangents;
layout (location = 4) in vec3 binormal;

uniform mat4 vertex_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform mat4 normal_model_to_world;
uniform float elapsed_time_s;

out VS_OUT {
	vec3 vertex;
	vec2 texcoord;
	vec3 normal;
	mat3 TBN;
} vs_out;


void main()
{
	vec3 vertex_pos = vertex;
	float offset = 0.1*cos(elapsed_time_s);
	float pivot_angle = 0.1*cos(elapsed_time_s);
	mat2 rotation_matrix = mat2(vec2(cos(pivot_angle), -sin(pivot_angle)), vec2(sin(pivot_angle), cos(pivot_angle)));
	float body = (vertex_pos.z + 1.0) / 2.0;
	float mask = smoothstep(0, 1, 1.0 - body);

	vertex_pos.x += offset;
	vertex_pos.xz = rotation_matrix * vertex_pos.xz;
	vertex_pos.x += cos(elapsed_time_s + body) * mask;


	vs_out.texcoord = texcoord.xy;
	vs_out.normal = normal;
	vs_out.vertex = vec3(vertex_model_to_world * vec4(vertex_pos, 1.0));

	vec3 T = normalize(vec3(vertex_model_to_world * vec4(tangents, 0.0)));
	vec3 B = normalize(vec3(vertex_model_to_world * vec4(binormal, 0.0)));
	vec3 N = normalize(vec3(normal_model_to_world * vec4(normal, 0.0)));
	vs_out.TBN = mat3(T, B, N);

	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex_pos, 1.0);
}
