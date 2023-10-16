#pragma once

#include "core/InputHandler.h"
#include "core/FPSCamera.h"
#include "core/WindowManager.hpp"
#include "core/node.hpp"


class Window;
namespace edaf80
{
	//! \brief Wrapper class for Assignment 5
	class Assignment5 {
	public:
		enum MovingStyle
		{
			CONTROL_POINTS,
			CIRCULAR,
			FOREWARD,
			BACKWARD,
			UPWARD,
			DOWNWARD
		};

		//! \brief Default constructor.
		//!
		//! It will initialise various modules of bonobo and retrieve a
		//! window to draw to.
		Assignment5(WindowManager& windowManager);

		//void moveObject(Node& Object, const std::vector<glm::vec3>& _control_point_locations,
		//	float duration_s, float elapsed_time_s, enum MovingStyle movingStyle);

		//! \brief Default destructor.
		//!
		//! It will release the bonobo modules initialised by the
		//! constructor, as well as the window.
		~Assignment5();

		//! \brief Contains the logic of the assignment, along with the
		//! render loop.
		void run();

		void moveObject(Node& Object, const std::vector<glm::vec3>& _control_point_locations,
			float duration_s, float elapsed_time_s, MovingStyle movingStyle);

	private:
		FPSCameraf     mCamera;
		InputHandler   inputHandler;
		WindowManager& mWindowManager;
		GLFWwindow*    window;
	};
}

