#version 410

uniform vec3 light_position;
uniform vec3 camera_position;

uniform vec3 ambient_colour;
uniform vec3 diffuse_colour;
uniform vec3 specular_colour;
uniform float shininess_value;
uniform mat4 normal_model_to_world;

uniform sampler2D water_normal_texture;
uniform samplerCube water_reflection_texture;
uniform bool use_normal_mapping;
vec3 N;
vec3 normalCoord0;
vec3 normalCoord1;
vec3 normalCoord2;
vec3 normal0;
vec3 normal1;
vec3 normal2;
vec3 normal;

vec4 reflect_text;

in VS_OUT {
	vec3 vertex;
	vec3 normal;
	vec2 text_coord;
	vec3 tangents;
	vec3 binormal;
	mat3 TBN;
	float time;
} fs_in;

out vec4 frag_color;

void main()
{
	vec4 color_deep = vec4(0.0f, 0.0f, 0.1f, 0.5f);
	vec4 color_shallow = vec4(0.0f, 0.5f, 0.5f, 0.5f);
	vec2 textScale = vec2(8.0f, 4.0f);
	float normalTime = mod(fs_in.time, 100.0f);
	vec2 normalSpeed = vec2(-0.05, 0.0);
	float Ro = 0.02037f;
	float eta = 0.75188f;
 
//	if(use_normal_mapping == true)
//	{
////		normalCoord0.xy = fs_in.text_coord.xy * textScale + normalTime * normalSpeed;
////		normalCoord1.xy = fs_in.text_coord.xy * textScale * 2.0 + normalTime * normalSpeed * 4.0;
////		normalCoord2.xy = fs_in.text_coord.xy * textScale * 4.0 + normalTime * normalSpeed * 8.0;
////		normal0 = texture(water_normal_texture, normalCoord0.xy).rgb * 2.0 - 1.0;
////		normal1 = texture(water_normal_texture, normalCoord1.xy).rgb * 2.0 - 1.0;
////		normal2 = texture(water_normal_texture, normalCoord2.xy).rgb * 2.0 - 1.0;
//		normal = texture(water_normal_texture, fs_in.text_coord.xy).rgb * 2.0 - 1.0;
//
//		N = normalize(fs_in.TBN * normal);
//	}
//	else
//	{
		N = normalize(fs_in.normal);
//	}


	vec3 L = normalize(light_position - fs_in.vertex); //Light vector
	vec3 V = normalize(camera_position - fs_in.vertex); //Viewer vector
	float facing = 1.0f - max(dot(V, N), 0.0f);
	vec4 water_color = mix(color_deep, color_shallow, facing);
	float fresnel = Ro + (1.0f - Ro) * pow((1.0f - dot(V, N)), 5.0f);
	vec4 reflection_color = texture(water_reflection_texture, reflect(-V,N).xyz);
	vec4 refraction_color = texture(water_reflection_texture, refract(-V,N, eta).xyz);

	frag_color = water_color + refraction_color * (1 - fresnel);


}
