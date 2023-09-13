#include "CelestialBody.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include "core/helpers.hpp"
#include "core/Log.h"

CelestialBody::CelestialBody(bonobo::mesh_data const& shape,
                             GLuint const* program,
                             GLuint diffuse_texture_id)
{
	_body.node.set_geometry(shape);
	_body.node.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
	_body.node.set_program(program);
}

glm::mat4 CelestialBody::render(std::chrono::microseconds elapsed_time,
                                glm::mat4 const& view_projection,
                                glm::mat4 const& parent_transform,
                                bool show_basis)
{
	// Convert the duration from microseconds to seconds.
	auto const elapsed_time_s = std::chrono::duration<float>(elapsed_time).count();
	// If a different ratio was needed, for example a duration in
	// milliseconds, the following would have been used:
	//auto const elapsed_time_ms = std::chrono::duration<float, std::milli>(elapsed_time).count();

	_body.spin.rotation_angle += elapsed_time_s * _body.spin.speed; //Ex 2.6

	glm::mat4 world = parent_transform;
	glm::mat4 children_transform(1.0f);
	glm::vec3 X_axis = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 Y_axis = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 Z_axis = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::mat4 I = glm::mat4(1.0f);
	glm::mat4 child_transform = glm::mat4(1.0f);
	int iNbrChild = 0;

	//Excercise 1
	//glm::vec3 V = glm::vec3(1.0f, 0.2f, 0.2f); //Scale factor - Flat //Ex 1.4
	//glm::vec3 V = glm::vec3(1.0f); //Scale factor /7Ex 1.3
	glm::vec3 scale = _body.scale;
	glm::mat4 S = glm::scale(I, scale);
	//world = S;
	//End excercise 1

	//Excercise 2
	glm::mat4 R1s = glm::rotate(I, _body.spin.rotation_angle, Y_axis); //Ex 2.1
	glm::mat4 R2s = glm::rotate(I, _body.spin.axial_tilt, Z_axis); //Ex 2.2
	//world = R1s; //Ex 2.3, 2.4, 2.5
	//End excercise 2

	//Excercise 3
	_body.orbit.rotation_angle += elapsed_time_s * _body.orbit.speed; //Ex 3.3
	glm::vec3 t = glm::vec3( _body.orbit.radius, 0.0f, 0.0f); //How to access the earth_orbit variable?

	glm::mat4 To = glm::translate(I, t); //Translate the orbit center
	//glm::mat4 dummy = glm::translate(I, glm::vec3(-2.5f, 0.0f, 0.0f)); //Translate the orbit center
	glm::mat4 R1o = glm::rotate(I, _body.orbit.rotation_angle, Y_axis); //Rotate around the orbit center
	glm::mat4 R2o = glm::rotate(I, _body.orbit.inclination, Z_axis); //Ex 3.5 - Tilt the orbit plane to the z-axis
	world = parent_transform * R2o * R1o * To * R2s * R1s * S ; //Ex 3.1, 3.2, 3.4 , 3.6, 4.1
	//world = R2s * R1s * S * parent_transform; //Ex 3.1, 3.2, 3.4 , 3.6, 4.1
	//End excercise 3

	if (show_basis)
	{
		bonobo::renderBasis(1.0f, 2.0f, view_projection, world);
	}

	// Note: The second argument of `node::render()` is supposed to be the
	// parent transform of the node, not the whole world matrix, as the
	// node internally manages its local transforms. However in our case we
	// manage all the local transforms ourselves, so the internal transform
	// of the node is just the identity matrix and we can forward the whole
	// world matrix.

	_body.node.render(view_projection, world);

	//Excercise 4: Calculate child's transformation
	children_transform = R2o * R1o *  To; //Apply the parent translation to the orbit and the orbit rotation + inclination
	//End excercise 4
	return children_transform;
}

void CelestialBody::add_child(CelestialBody* child)
{
	_children.push_back(child);
}

std::vector<CelestialBody*> const& CelestialBody::get_children() const
{
	return _children;
}

void CelestialBody::set_orbit(OrbitConfiguration const& configuration)
{
	_body.orbit.radius = configuration.radius;
	_body.orbit.inclination = configuration.inclination;
	_body.orbit.speed = configuration.speed;
	_body.orbit.rotation_angle = 0.0f;
}

void CelestialBody::set_scale(glm::vec3 const& scale)
{
	_body.scale = scale;
}

void CelestialBody::set_spin(SpinConfiguration const& configuration)
{
	_body.spin.axial_tilt = configuration.axial_tilt;
	_body.spin.speed = configuration.speed;
	_body.spin.rotation_angle = 0.0f;
}

void CelestialBody::set_ring(bonobo::mesh_data const& shape,
                             GLuint const* program,
                             GLuint diffuse_texture_id,
                             glm::vec2 const& scale)
{
	_ring.node.set_geometry(shape);
	_ring.node.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
	_ring.node.set_program(program);

	_ring.scale = scale;

	_ring.is_set = true;
}
