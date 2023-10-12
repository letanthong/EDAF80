#pragma once

#include "core/InputHandler.h"
#include "core/FPSCamera.h"
#include "core/WindowManager.hpp"
#include "core/node.hpp"


class Window;

enum IntepolateMethod
{
	LINEAR,
	CATTROM
};
namespace edaf80
{
	//! \brief Wrapper class for Assignment 5
	class Assignment5 {
	public:
		//! \brief Default constructor.
		//!
		//! It will initialise various modules of bonobo and retrieve a
		//! window to draw to.
		Assignment5(WindowManager& windowManager);

		void moveObject(Node& Object, const std::vector<glm::vec3>& _control_point_locations,
			float duration_s, float elapsed_time_s, enum IntepolateMethod method);

		//! \brief Default destructor.
		//!
		//! It will release the bonobo modules initialised by the
		//! constructor, as well as the window.
		~Assignment5();

		//! \brief Contains the logic of the assignment, along with the
		//! render loop.
		void run();

	private:
		FPSCameraf     mCamera;
		InputHandler   inputHandler;
		WindowManager& mWindowManager;
		GLFWwindow*    window;
	};
}

namespace movement
{
	//Moving an object along a list of control points within a predefined duration
	
}
