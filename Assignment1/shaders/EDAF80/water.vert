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
	mat3 TBN;
} vs_out;

float wave(in float Amp,in vec2 Di, in vec2 textCoord, in float freq, in float phase, in float sharpness, in float time)
{
	float alpha = 0.5*sin((Di.x * textCoord.x + Di.y * textCoord.y)*freq + time*phase) + 0.5;
	return Amp * pow(alpha, sharpness);
}

void main()
{
	vs_out.vertex = vec3(vertex_model_to_world * vec4(vertex, 1.0));
	float wave_1 = wave(1.0f, vec2(-1.0f, 0.0f), vs_out.vertex.xz, 0.2f, 0.5f, 2.0f, elapsed_time_s );
	float wave_2 = wave(0.5f, vec2(-0.7f, 0.7f), vs_out.vertex.xz, 0.4f, 1.3f, 2.0f, elapsed_time_s );
	vs_out.vertex.y = wave_1 + wave_2;
	
	vs_out.normal = vec3(normal_model_to_world * vec4(normal, 0.0));
	vs_out.text_coord = text_coord.xz;

	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex, 1.0);
}
