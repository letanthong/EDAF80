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

void wave(in float Amp,in vec3 Di, in float freq, in float phase, in float sharpness, in float time,
			inout vec3 vertex, inout vec3 Grad)
{
	float angle = (Di.x * vertex.x + Di.z * vertex.z)*freq + time*phase;
	float alpha = 0.5*sin(angle) + 0.5;
	vertex.y += Amp * pow(alpha, sharpness); //Sum all the wave vertex y
	Grad.x += 0.5 * sharpness * freq * Amp * pow(alpha, (sharpness - 1.0f)) * cos(angle)* Di.x;
	Grad.z += 0.5 * sharpness * freq * Amp * pow(alpha, (sharpness - 1.0f)) * cos(angle)* Di.z;
}

void main()
{
	vec3 Grad = vec3(0.0f); //Gradient of wave
	vec3 Di_1 = vec3(-1.0f, 0.0f, 0.0f);
	vec3 Di_2 = vec3(-0.7f, 0.0f, 0.7f);
	float Amp1 = 1.0f;
	float Amp2 = 0.5f;
	float freq1 = 0.2f;
	float freq2 = 0.4f;
	float phase1 = 0.5f;
	float phase2 = 1.3f;
	float sharpness1 = 2.0f;
	float sharpness2 = 2.0f;

	vec3 displace_vertex = vertex;
	wave(Amp1, Di_1, freq1, phase1, sharpness1, elapsed_time_s , displace_vertex, Grad);
	wave(Amp2, Di_2, freq2, phase2, sharpness2, elapsed_time_s , displace_vertex, Grad);

	vs_out.vertex = vec3(vertex_model_to_world * vec4(displace_vertex, 1.0f));
	vs_out.normal = vec3(normal_model_to_world * vec4(vec3( -Grad.x, 1.0f, -Grad.z ), 1.0f));
	vs_out.text_coord = text_coord.xz;
	vs_out.tangents = vec3(vertex_model_to_world * vec4(vec3( 1.0f, Grad.x, 0.0f ), 1.0f));
	vs_out.binormal = vec3(vertex_model_to_world * vec4(vec3( 0.0f, Grad.z, 1.0f), 1.0f));
	vs_out.time = elapsed_time_s;

//	vec3 T = normalize(vec3(vertex_model_to_world * vec4(vs_out.tangents, 0.0)));
//	vec3 B = normalize(vec3(vertex_model_to_world * vec4(vs_out.binormal, 0.0)));
//	vec3 N = normalize(vec3(normal_model_to_world * vec4(vs_out.normal, 0.0)));
	vec3 T = normalize(vs_out.tangents);
	vec3 B = normalize(vs_out.binormal);
	vec3 N = normalize(vs_out.normal);
	vs_out.TBN = mat3(T, B, N);

	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(displace_vertex, 1.0f);
}
