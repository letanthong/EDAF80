#include "movement.hpp"
#include "core/node.hpp"



void moveObject(Node& Object, std::vector<glm::vec3>& control_point_locations, float duration_s, float elapsed_time_s, enum InterpolationMethod method)
{
	int numb_cp = control_point_locations.size();
	glm::vec3 newLoc;
	int i = 0;

	if (LINEAR == method) {
		float x = fmod(elapsed_time_s, duration_s);
		int index = static_cast<int> (elapsed_time_s / duration_s);
		glm::vec3 p0 = control_point_locations[index % numb_cp];
		glm::vec3 p1 = control_point_locations[(index + 1) % numb_cp];
		newLoc = interpolation::evalLERP(p0, p1, x);
		Object.get_transform().SetTranslate(newLoc);
	}
	else {
		float x = fmod(elapsed_time_s, duration_s);
		int index = static_cast<int> (elapsed_time_s / duration_s);
		float catmull_rom_tension = 0.5f;

		glm::vec3 p0 = control_point_locations[index % numb_cp];
		glm::vec3 p1 = control_point_locations[(index + 1) % numb_cp];
		glm::vec3 p2 = control_point_locations[(index + 2) % numb_cp];
		glm::vec3 p3 = control_point_locations[(index + 3) % numb_cp];

		newLoc = interpolation::evalCatmullRom(p0, p1, p2, p3, catmull_rom_tension, x);
		Object.get_transform().SetTranslate(newLoc);

	}
}
