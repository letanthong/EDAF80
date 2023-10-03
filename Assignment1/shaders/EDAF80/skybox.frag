#version 410

in VS_OUT {
	vec3 skyboxcoord;
} fs_in;

uniform samplerCube image;
out vec4 frag_color;

void main()
{
	frag_color = texture(image, fs_in.skyboxcoord);
}
