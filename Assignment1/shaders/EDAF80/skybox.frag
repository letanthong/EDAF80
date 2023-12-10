#version 410

in VS_OUT {
	vec3 skyboxcoord;
} fs_in;

uniform samplerCube image;
out vec4 frag_color;

void main()
{
	float light_factor = 0.5;
	frag_color = light_factor * texture(image, fs_in.skyboxcoord);
//	frag_color = vec4(1.0f);
}
