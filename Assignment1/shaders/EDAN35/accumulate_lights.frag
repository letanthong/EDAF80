#version 410

struct ViewProjTransforms
{
	mat4 view_projection;
	mat4 view_projection_inverse;
};

layout (std140) uniform CameraViewProjTransforms
{
	ViewProjTransforms camera;
};

layout (std140) uniform LightViewProjTransforms
{
	ViewProjTransforms lights[4];
};

uniform int light_index;

uniform sampler2D depth_texture;
uniform sampler2D normal_texture;
uniform sampler2D shadow_texture;

uniform vec2 inverse_screen_resolution;

uniform vec3 camera_position;

uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3 light_direction;
uniform float light_intensity;
uniform float light_angle_falloff;

layout (location = 0) out vec4 light_diffuse_contribution;
layout (location = 1) out vec4 light_specular_contribution;


void main()
{
	vec2 shadowmap_texel_size = 1.0f / textureSize(shadow_texture, 0);

	//light_diffuse_contribution  = vec4(0.0, 0.0, 0.0, 1.0);
	//light_specular_contribution = vec4(0.0, 0.0, 0.0, 1.0);

	// Sample depth and normal from textures
    vec3 normal = texture(normal_texture, gl_FragCoord.xy).xyz;
	float depth = texture(depth_texture, gl_FragCoord.xy).r;

	vec4 screen_space_position = vec4(gl_FragCoord.xy * inverse_screen_resolution * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);

    // Perform the inverse projection using the camera.view_projection_inverse matrix
    vec4 world_space_position = camera.view_projection_inverse * screen_space_position;

    // Divide by w to get homogeneous coordinates
    world_space_position /= world_space_position.w;

    // Phong shading calculations
    vec3 light_dir;
	float attenuation = 1.0;

    if (light_index == 0) {
        // Directional light
        light_dir = normalize(-light_direction);
    } else {
        // Point light
        light_dir = normalize(light_position - world_space_position.xyz);

		float distance = length(light_dir);
        attenuation = 1.0 / (1.0 + 0.1 * distance + 0.01 * distance * distance);

        // Spotlight with angular falloff
        float cos_angle = dot(normalize(-light_dir), normalize(light_direction));
        float spotlight_factor = smoothstep(light_angle_falloff, 1.0, cos_angle);
        
        // Combine distance and angular falloff
        attenuation *= spotlight_factor;
        
        light_dir = normalize(light_dir);
    }

    vec3 view_direction = normalize( camera_position -world_space_position.xyz);
	vec3 N = normalize(normal);
    vec3 reflect_dir = reflect(-light_dir, N);

    // Diffuse and specular components
    float diffuse = max(dot(normal, light_dir), 0.0);
    float specular = pow(max(dot(view_direction, reflect_dir), 0.0), light_intensity);

    // Combine ambient, diffuse, and specular
    vec3 diffuse_color = light_color * diffuse * attenuation;
    vec3 specular_color = light_color * specular * attenuation;

    // Output the final contributions
    light_diffuse_contribution = vec4(diffuse_color, 1.0);
    light_specular_contribution = vec4(specular_color, 1.0);
}
