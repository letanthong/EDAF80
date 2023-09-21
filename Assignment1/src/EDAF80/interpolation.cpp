#include "interpolation.hpp"

glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
	//! \todo Implement this function
	glm::vec2 t_trans = glm::vec2( 1.0f, x);
	glm::mat2 M_temp = glm::mat2( glm::vec2(1.0f, 0.0f), glm::vec2(-1.0f, 1.0f) );
	glm::mat2 M_inverse = glm::transpose(M_temp);
	glm::mat2x3 p_temp = glm::mat2x3(p0, p1);
	glm::mat3x2 p = glm::transpose(p_temp);

	glm::vec3 p_x = t_trans * M_inverse * p;

	return p_x;
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
                              glm::vec3 const& p2, glm::vec3 const& p3,
                              float const t, float const x)
{
	//! \todo Implement this function
	glm::vec4 t_trans = glm::vec4(1.0f, x, glm::pow(x,2.0f), glm::pow(x,3.0f));
	glm::mat4 M_temp = glm::mat4(glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
									glm::vec4(-1.0f*t, 0.0f, t, 0.0f),
									glm::vec4(2.0f*t, t-3, 3-2.0f*t, -t),
									glm::vec4(-1.0f*t, 2-t, t-2, t) );
	glm::mat4 M_inverse = glm::transpose(M_temp);
	glm::mat4x3 p_temp = glm::mat4x3(p0, p1, p2, p3);
	glm::mat3x4 p = glm::transpose(p_temp);

	glm::vec3 p_x = t_trans * M_inverse * p;
	

	return p_x;
}
