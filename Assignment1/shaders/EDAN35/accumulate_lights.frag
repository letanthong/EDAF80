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

//// Function to perform PCF sampling
//float pcfSample(sampler2D shadowMap, vec2 shadowCoords, float compareDepth, float radius)
//{
//    float result = 0.0;
//    int sampleCount = 4; // You can adjust this for more or fewer samples
//
//    for (int i = -sampleCount; i <= sampleCount; ++i)
//    {
//        for (int j = -sampleCount; j <= sampleCount; ++j)
//        {
//            vec2 offset = vec2(float(i), float(j)) * radius;
//            float depth = texture(shadowMap, shadowCoords + offset).r;
//            result += (depth + radius < compareDepth) ? 1.0 : 0.0;
//        }
//    }
//
//    // Average the results
//    return result / float((2 * sampleCount + 1) * (2 * sampleCount + 1));
//}
//
void main()
{
	vec2 shadowmap_texel_size = 1.0f / textureSize(shadow_texture, 0);

	//light_diffuse_contribution  = vec4(0.0, 0.0, 0.0, 1.0);
	//light_specular_contribution = vec4(0.0, 0.0, 0.0, 1.0);
	vec2 textCoord =gl_FragCoord.xy * inverse_screen_resolution;
	// Sample depth and normal and shadow from textures
	vec3 normal = texture(normal_texture, textCoord ).xyz; //need to translate to -1 1
	float depth = texture(depth_texture,textCoord).r ;
	//float shadowPos = texture(shadow_texture, gl_FragCoord.xy).r;

	//***Calculate worldspace postion***//
	vec4 screen_space_position = vec4(textCoord*2.0-1.0, depth * 2.0 - 1.0, 1.0);
//	vec4 screen_space_position = vec4((gl_FragCoord.xy * inverse_screen_resolution) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);

    // Perform the inverse projection using the camera.view_projection_inverse matrix
    vec4 world_space_position = camera.view_projection_inverse * screen_space_position;
    // Divide by w to get homogeneous coordinates
    world_space_position /= world_space_position.w;

	//*****************************//
    //***Calculate Phong shading***//
    vec3 light_dir;
	float attenuation = 1.0;
	float light_distance = 0.0;
	vec3 view_direction;
	vec3 reflect_dir;

//    if (light_index == 0) {
//        // Directional light
//        light_dir = normalize(-light_direction);
//    } else {
        // Point light
        light_dir = normalize(light_position - world_space_position.xyz);
		light_distance = length(light_position - world_space_position.xyz);
        float distance_falloff = 1.0 / (1.0  + light_distance * light_distance);
//        float distance_falloff = 1.0 / (light_distance * light_distance);

        // Spotlight with angular falloff
        float theta = dot(light_dir, normalize(-light_direction));
		float epsilon = 0.09;
//        float angular_falloff = smoothstep(light_angle_falloff, 1.0, cos_angle);
        float angular_falloff =max( clamp(theta - cos(light_angle_falloff), 0.0, 1.0)/(1.0-cos(light_angle_falloff)),0);
        
        // Combine distance and angular falloff
        attenuation = distance_falloff * angular_falloff;
//    }

    view_direction = normalize( camera_position -world_space_position.xyz);
	vec3 N = normal*2.0 - 1.0;
	N=normalize(N);
    reflect_dir = reflect(-light_dir, N);

    // Diffuse and specular components
    float diffuse = max(dot(N, light_dir), 0.0);
    float specular = pow(max(dot(view_direction, reflect_dir), 0.0), 32.0);

    // Combine ambient, diffuse, and specular
    vec3 diffuse_color = light_color * diffuse * attenuation * light_intensity;
    vec3 specular_color = light_color * specular * attenuation * light_intensity;

	// Check if the pixel is in shadow using PCF
    float shadow_factor = 1.0;
	
    // Use lights[light_index].view_projection to project the fragment's position into the shadow map
    vec4 frag_light_space = lights[light_index].view_projection * world_space_position;
    // Normalize homogeneous coordinates and convert to [0, 1] range
    vec3 shadow_coords = frag_light_space.xyz / frag_light_space.w;
	shadow_coords=shadow_coords*0.5+0.5;
	float shadow_depth=texture(shadow_texture,shadow_coords.xy).r;
	if(shadow_coords.z > (shadow_depth+0.000001))
	{
		shadow_factor = 0.0;
	}

	float temp_shadow;
	float sum_shadow = 0.0;
	for (int i = -1; i < 2; i++)
	{
		for(int j = -1; j < 2; j++)
		{
		vec2 sum_textcoord = shadow_coords.xy;
		float shadow_a = texture(shadow_texture, shadow_coords.xy + vec2(i,j) * shadowmap_texel_size).x + 0.0001;
		if(shadow_coords.z > shadow_a)
		{
			temp_shadow = 0.0;
		}
		else
		{
			temp_shadow = 1.0;
		}
			sum_shadow += temp_shadow;
		}
	}
	shadow_factor = sum_shadow/9;
    // Output the final contributions
    light_diffuse_contribution = shadow_factor*vec4(diffuse_color, 1.0);
    light_specular_contribution =shadow_factor* vec4(specular_color, 1.0);
}
