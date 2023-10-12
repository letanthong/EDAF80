#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <core/node.hpp>
#include "interpolation.hpp"

namespace movement
{
	//Moving an object along a list of control points within a predefined duration
	void moveObject(Node& Object, const std::vector<glm::vec3> &control_point_locations,
		float duration_s, float elapsed_time_s, enum InterpolationMethod method) {};
};
