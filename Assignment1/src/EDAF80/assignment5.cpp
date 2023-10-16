#include "assignment5.hpp"
#include "parametric_shapes.hpp"
#include "interpolation.hpp"

#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <tinyfiledialogs.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <clocale>
#include <stdexcept>
#include <cstdlib>
#include <vector>

edaf80::Assignment5::Assignment5(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 5", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}

	bonobo::init();
}

edaf80::Assignment5::~Assignment5()
{
	bonobo::deinit();
}

void
edaf80::Assignment5::run()
{
	// Set up the camera
	mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	mCamera.mWorld.LookAt(glm::vec3(0.0f));
	mCamera.mMouseSensitivity = glm::vec2(0.003f);
	mCamera.mMovementSpeed = glm::vec3(3.0f); // 3 m/s => 10.8 km/h
	auto camera_position = mCamera.mWorld.GetTranslation();

	// Create the shader programs
	ShaderProgramManager program_manager;
	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fallback",
	                                         { { ShaderType::vertex, "common/fallback.vert" },
	                                           { ShaderType::fragment, "common/fallback.frag" } },
	                                         fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}

	GLuint diffuse_shader = 0u;
	program_manager.CreateAndRegisterProgram("Diffuse",
		{ { ShaderType::vertex, "EDAF80/diffuse.vert" },
		  { ShaderType::fragment, "EDAF80/diffuse.frag" } },
		diffuse_shader);
	if (diffuse_shader == 0u)
		LogError("Failed to load diffuse shader");

	GLuint normal_shader = 0u;
	program_manager.CreateAndRegisterProgram("Normal",
		{ { ShaderType::vertex, "EDAF80/normal.vert" },
		  { ShaderType::fragment, "EDAF80/normal.frag" } },
		normal_shader);
	if (normal_shader == 0u)
		LogError("Failed to load normal shader");

	GLuint texcoord_shader = 0u;
	program_manager.CreateAndRegisterProgram("Texture coords",
		{ { ShaderType::vertex, "EDAF80/texcoord.vert" },
		  { ShaderType::fragment, "EDAF80/texcoord.frag" } },
		texcoord_shader);
	if (texcoord_shader == 0u)
		LogError("Failed to load textcoord shader");

	GLuint skybox_shader = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
		{ { ShaderType::vertex, "EDAF80/skybox.vert" },
		  { ShaderType::fragment, "EDAF80/skybox.frag" } },
		skybox_shader
	);

	if (texcoord_shader == 0u)
		LogError("Failed to load textcoord shader");

	//tuna shader
	GLuint tuna_shader = 0u;
	program_manager.CreateAndRegisterProgram("Tuna",
		{ { ShaderType::vertex, "EDAF80/tuna.vert" },
		  { ShaderType::fragment, "EDAF80/tuna.frag" } },
		tuna_shader
	);
	if (tuna_shader == 0u)
		LogError("Failed to load tuna shader");

	GLuint submarine_shader = 0u;
	program_manager.CreateAndRegisterProgram("Submarine",
		{ { ShaderType::vertex, "EDAF80/submarine.vert" },
		  { ShaderType::fragment, "EDAF80/submarine.frag" } },
		submarine_shader
	);
	if (submarine_shader == 0u)
		LogError("Failed to load submarine shader");

	GLuint shark_shader = 0u;
	program_manager.CreateAndRegisterProgram("Shark",
		{ { ShaderType::vertex, "EDAF80/shark.vert" },
		  { ShaderType::fragment, "EDAF80/shark.frag" } },
		shark_shader
	);
	if (submarine_shader == 0u)
		LogError("Failed to load shark shader");
	

	auto light_position = glm::vec3(-2.0f, 4.0f, 2.0f);
	auto const set_uniforms = [&light_position](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position)); };

	//
	// Todo: Insert the creation of other shader programs.
	//       (Check how it was done in assignment 3.)
	//

	//
	// Todo: Load your geometry
	//
	//
	//
	//
	auto skybox_shape = parametric_shapes::createSphere(100.0f, 1000u, 1000u);
	if (skybox_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}

	//Test
	auto test_point = parametric_shapes::createSphere(1.0f, 10u, 10u);
	if (test_point.vao == 0u) {
		LogError("Failed to retrieve the mesh for the test point");
		return;
	}
	Node test;
	test.set_geometry(test_point);
	//End test

	GLuint const skybox_texture = bonobo::loadTextureCubeMap(
		config::resources_path("cubemaps/Underwater/uw_ft_posx.jpg"),
		config::resources_path("cubemaps/Underwater/uw_bk_negx.jpg"),
		config::resources_path("cubemaps/Underwater/uw_up_posy.jpg"),
		config::resources_path("cubemaps/Underwater/uw_dn_negy.jpg"),
		config::resources_path("cubemaps/Underwater/uw_rt_posz.jpg"),
		config::resources_path("cubemaps/Underwater/uw_lf_negz.jpg"),
		true
	);

	Node skybox;
	skybox.set_geometry(skybox_shape);
	skybox.add_texture("skybox_texture", skybox_texture, GL_TEXTURE_CUBE_MAP);
	skybox.set_program(&skybox_shader, set_uniforms);

	//Tuna
	bonobo::material_data tuna_material;
	tuna_material.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	tuna_material.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
	tuna_material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	tuna_material.shininess = 10.0f;

	std::vector<bonobo::mesh_data> tuna_model;
	tuna_model = bonobo::loadObjects(config::resources_path("textures/tuna/tuna_fish.obj"));
	GLuint const tuna_body_diff = bonobo::loadTexture2D(config::resources_path("textures/tuna/tuna_body_diff.png"), true);
	GLuint const tuna_body_rough = bonobo::loadTexture2D(config::resources_path("textures/tuna/tuna_body_rough.png"), true);
	GLuint const tuna_body_normal = bonobo::loadTexture2D(config::resources_path("textures/tuna/tuna_body_normal.png"), true);

	Node tuna;
	tuna.set_geometry(tuna_model.at(0));
	tuna.add_texture("tuna_body_diff", tuna_body_diff, GL_TEXTURE_2D);
	tuna.add_texture("tuna_body_rough", tuna_body_rough, GL_TEXTURE_2D);
	tuna.add_texture("tuna_body_normal", tuna_body_normal, GL_TEXTURE_2D);
	tuna.set_material_constants(tuna_material);

	//Submarine
	bonobo::material_data submarine_material;
	submarine_material.ambient = glm::vec3(0.3f, 0.3f, 0.3f);
	submarine_material.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
	submarine_material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	submarine_material.shininess = 10.0f;

	std::vector<bonobo::mesh_data> submarine_model;
	submarine_model = bonobo::loadObjects(config::resources_path("textures/submarine/submarine.obj"));
	GLuint const submarine_diff = bonobo::loadTexture2D(config::resources_path("textures/submarine/submarine_diff.png"), true);
	GLuint const submarine_rough = bonobo::loadTexture2D(config::resources_path("textures/submarine/submarine_spec.png"), true);
	GLuint const submarine_normal = bonobo::loadTexture2D(config::resources_path("textures/submarine/submarine_normal.png"), true);

	Node submarine;
	submarine.set_geometry(submarine_model.at(0));
	submarine.add_texture("submarine_diff", submarine_diff, GL_TEXTURE_2D);
	submarine.add_texture("submarine_rough", submarine_rough, GL_TEXTURE_2D);
	submarine.add_texture("submarine_normal", submarine_normal, GL_TEXTURE_2D);
	submarine.set_material_constants(submarine_material);
	submarine.get_transform().SetScale(0.01f);
	//End submarine

	//Shark
	bonobo::material_data shark_material;
	shark_material.ambient = glm::vec3(0.5f, 0.5f, 0.5f);
	shark_material.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
	shark_material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	shark_material.shininess = 10.0f;

	std::vector<bonobo::mesh_data> shark_model;
	shark_model = bonobo::loadObjects(config::resources_path("textures/shark/shark.obj"));
	GLuint const shark_diff = bonobo::loadTexture2D(config::resources_path("textures/shark/shark_diff.png"), true);
	GLuint const shark_rough = bonobo::loadTexture2D(config::resources_path("textures/shark/shark_rough.png"), true);
	GLuint const shark_normal = bonobo::loadTexture2D(config::resources_path("textures/shark/shark_normal.png"), true);

	Node shark;
	shark.set_geometry(shark_model.at(0));
	shark.add_texture("shark_diff", shark_diff, GL_TEXTURE_2D);
	shark.add_texture("shark_rough", shark_rough, GL_TEXTURE_2D);
	shark.add_texture("shark_normal", shark_normal, GL_TEXTURE_2D);
	shark.set_material_constants(shark_material);
	shark.get_transform().SetScale(0.5f);
	//End shark

	//Treasure
	bonobo::material_data treasure_material;
	treasure_material.ambient = glm::vec3(0.5f, 0.5f, 0.5f);
	treasure_material.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
	treasure_material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	treasure_material.shininess = 10.0f;

	std::vector<bonobo::mesh_data> treasure_model;
	treasure_model = bonobo::loadObjects(config::resources_path("textures/treasure/treasure.obj"));
	GLuint const treasure_diff = bonobo::loadTexture2D(config::resources_path("textures/treasure/treasure_diff.png"), true);
	GLuint const treasure_rough = bonobo::loadTexture2D(config::resources_path("textures/treasure/treasure_spec.png"), true);
	GLuint const treasure_normal = bonobo::loadTexture2D(config::resources_path("textures/treasure/treasure_normal.png"), true);

	Node treasure;
	treasure.set_geometry(treasure_model.at(0));
	treasure.add_texture("shark_diff", treasure_diff, GL_TEXTURE_2D);
	treasure.add_texture("shark_rough", treasure_rough, GL_TEXTURE_2D);
	treasure.add_texture("shark_normal", treasure_normal, GL_TEXTURE_2D);
	treasure.set_material_constants(treasure_material);
	treasure.get_transform().SetScale(0.1f);
	//End treasure

	//Coin
	bonobo::material_data coin_material;
	coin_material.ambient = glm::vec3(0.5f, 0.5f, 0.5f);
	coin_material.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
	coin_material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	coin_material.shininess = 10.0f;

	std::vector<bonobo::mesh_data> coin_model;
	coin_model = bonobo::loadObjects(config::resources_path("textures/treasure/coin.obj"));
	GLuint const coin_diff = bonobo::loadTexture2D(config::resources_path("textures/treasure/coin_diff.png"), true);
	GLuint const coin_rough = bonobo::loadTexture2D(config::resources_path("textures/treasure/coin_spec.png"), true);
	GLuint const coin_normal = bonobo::loadTexture2D(config::resources_path("textures/treasure/coin_normal.png"), true);
	//End coin

	//Control points
	//auto const control_point_sphere = parametric_shapes::createSphere(0.1f, 10u, 10u);
	std::vector <glm::vec3> control_point_locations = {
		glm::vec3(10.0f,  10.0f,  10.0f),
		glm::vec3(10.0f,  18.0f,  10.0f),
		glm::vec3(20.0f,  12.0f,  20.0f),
		glm::vec3(20.0f,  15.0f,  20.0f),
		glm::vec3(15.0f,  0.0f,  15.0f),
		glm::vec3(-20.0f, -10.0f,  15.0f),
		glm::vec3(-15.0f, -15.0f, -15.0f),
		glm::vec3(-20.0f, -12.0f, -20.0f),
		glm::vec3(-10.0f, -18.0f, -10.0f)
	};

	std::array<Node, 9> control_points;
	for (std::size_t i = 0; i < control_point_locations.size(); ++i) {
		auto& control_point = control_points[i];
		control_point.set_geometry(coin_model.at(0));
		control_point.set_program(&diffuse_shader, set_uniforms);
		control_point.get_transform().SetTranslate(control_point_locations[i]);
		control_point.set_material_constants(coin_material);
	}
	//End control points
	
	

	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);


	float elapsed_time_s = 0.0f;
	bool pause_animation = false;
	bool use_orbit_camera = false;
	auto lastTime = std::chrono::high_resolution_clock::now();

	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;
	bool show_basis = false; /*Red - x, Green - y, Blue - z*/
	float basis_thickness_scale = 1.0f;
	float basis_length_scale = 1.0f;

	auto cull_mode = bonobo::cull_mode_t::disabled;
	auto polygon_mode = bonobo::polygon_mode_t::fill;
	bool use_normal_mapping = true;
	bool interpolate = false;
	bool show_control_points = true;

	while (!glfwWindowShouldClose(window)) {
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		if (!pause_animation) {
			elapsed_time_s += std::chrono::duration<float>(deltaTimeUs).count();
		}

		//tuna uniform
		auto const tuna_set_uniforms =
			[&use_normal_mapping, &light_position, &camera_position, &tuna_material, &elapsed_time_s](GLuint program) {
			glUniform1i(glGetUniformLocation(program, "use_normal_mapping"), use_normal_mapping ? 1 : 0);
			glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
			glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
			glUniform3fv(glGetUniformLocation(program, "ambient_colour"), 1, glm::value_ptr(tuna_material.ambient));
			glUniform3fv(glGetUniformLocation(program, "diffuse_colour"), 1, glm::value_ptr(tuna_material.diffuse));
			glUniform3fv(glGetUniformLocation(program, "specular_colour"), 1, glm::value_ptr(tuna_material.specular));
			glUniform1f(glGetUniformLocation(program, "shininess"), tuna_material.shininess);
			glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
			};
		tuna.set_program(&tuna_shader, tuna_set_uniforms);

		//submarine uniform
		auto const submarine_set_uniforms =
			[&use_normal_mapping, &light_position, &camera_position, &submarine_material, &elapsed_time_s](GLuint program) {
			glUniform1i(glGetUniformLocation(program, "use_normal_mapping"), use_normal_mapping ? 1 : 0);
			glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
			glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
			glUniform3fv(glGetUniformLocation(program, "ambient_colour"), 1, glm::value_ptr(submarine_material.ambient));
			glUniform3fv(glGetUniformLocation(program, "diffuse_colour"), 1, glm::value_ptr(submarine_material.diffuse));
			glUniform3fv(glGetUniformLocation(program, "specular_colour"), 1, glm::value_ptr(submarine_material.specular));
			glUniform1f(glGetUniformLocation(program, "shininess"), submarine_material.shininess);
			glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
			};
		submarine.set_program(&submarine_shader, submarine_set_uniforms);

		//shark uniform
		auto const shark_set_uniforms =
			[&use_normal_mapping, &light_position, &camera_position, &shark_material, &elapsed_time_s](GLuint program) {
			glUniform1i(glGetUniformLocation(program, "use_normal_mapping"), use_normal_mapping ? 1 : 0);
			glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
			glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
			glUniform3fv(glGetUniformLocation(program, "ambient_colour"), 1, glm::value_ptr(shark_material.ambient));
			glUniform3fv(glGetUniformLocation(program, "diffuse_colour"), 1, glm::value_ptr(shark_material.diffuse));
			glUniform3fv(glGetUniformLocation(program, "specular_colour"), 1, glm::value_ptr(shark_material.specular));
			glUniform1f(glGetUniformLocation(program, "shininess"), shark_material.shininess);
			glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
			};
		shark.set_program(&shark_shader, shark_set_uniforms);

		//treasure uniform
		auto const treasure_set_uniforms =
			[&use_normal_mapping, &light_position, &camera_position, &treasure_material, &elapsed_time_s](GLuint program) {
			glUniform1i(glGetUniformLocation(program, "use_normal_mapping"), use_normal_mapping ? 1 : 0);
			glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
			glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
			glUniform3fv(glGetUniformLocation(program, "ambient_colour"), 1, glm::value_ptr(treasure_material.ambient));
			glUniform3fv(glGetUniformLocation(program, "diffuse_colour"), 1, glm::value_ptr(treasure_material.diffuse));
			glUniform3fv(glGetUniformLocation(program, "specular_colour"), 1, glm::value_ptr(treasure_material.specular));
			glUniform1f(glGetUniformLocation(program, "shininess"), treasure_material.shininess);
			glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
			};
		treasure.set_program(&shark_shader, treasure_set_uniforms);

		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(deltaTimeUs, inputHandler);
		if (use_orbit_camera) {
			mCamera.mWorld.LookAt(glm::vec3(0.0f));
		}
		camera_position = mCamera.mWorld.GetTranslation();

		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
				                   "An error occurred while reloading shader programs; see the logs for details.\n"
				                   "Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
				                   "error");
		}
		if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F11) & JUST_RELEASED)
			mWindowManager.ToggleFullscreenStatusForWindow(window);


		// Retrieve the actual framebuffer size: for HiDPI monitors,
		// you might end up with a framebuffer larger than what you
		// actually asked for. For example, if you ask for a 1920x1080
		// framebuffer, you might get a 3840x2160 one instead.
		// Also it might change as the user drags the window between
		// monitors with different DPIs, or if the fullscreen status is
		// being toggled.
		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);


		//
		// Todo: If you need to handle inputs, you can do it here
		//

		float fMovingSpeed = 0.1f;
		//Left turn
		if ( (inputHandler.GetKeycodeState(GLFW_KEY_LEFT) & JUST_PRESSED) || (inputHandler.GetKeycodeState(GLFW_KEY_LEFT) & PRESSED) )
		{
			glm::vec3 SubNewLoc = submarine.get_transform().GetTranslation();
			SubNewLoc.x -= fMovingSpeed;
			submarine.get_transform().SetTranslate(SubNewLoc);
		}

		//Right turn
		if ( (inputHandler.GetKeycodeState(GLFW_KEY_RIGHT) & JUST_PRESSED) || (inputHandler.GetKeycodeState(GLFW_KEY_RIGHT) & PRESSED) )
		{
			glm::vec3 SubNewLoc = submarine.get_transform().GetTranslation();
			SubNewLoc.x += fMovingSpeed;
			submarine.get_transform().SetTranslate(SubNewLoc);
		}

		//Up
		if ( (inputHandler.GetKeycodeState(GLFW_KEY_UP) & JUST_PRESSED || inputHandler.GetKeycodeState(GLFW_KEY_UP) & PRESSED) )
		{
			glm::vec3 SubNewLoc = submarine.get_transform().GetTranslation();
			SubNewLoc.y += fMovingSpeed;
			submarine.get_transform().SetTranslate(SubNewLoc);
		}

		//Down
		if ( (inputHandler.GetKeycodeState(GLFW_KEY_DOWN) & JUST_PRESSED || inputHandler.GetKeycodeState(GLFW_KEY_DOWN) & PRESSED) )
		{
			glm::vec3 SubNewLoc = submarine.get_transform().GetTranslation();
			SubNewLoc.y -= fMovingSpeed;
			submarine.get_transform().SetTranslate(SubNewLoc);
		}

		//forward
		if ( (inputHandler.GetKeycodeState(GLFW_KEY_W) & JUST_PRESSED || inputHandler.GetKeycodeState(GLFW_KEY_W) & PRESSED) )
		{
			glm::vec3 SubNewLoc = submarine.get_transform().GetTranslation();
			SubNewLoc.z -= fMovingSpeed;
			submarine.get_transform().SetTranslate(SubNewLoc);
		}

		//backward
		if ( (inputHandler.GetKeycodeState(GLFW_KEY_S) & JUST_PRESSED || inputHandler.GetKeycodeState(GLFW_KEY_S) & PRESSED) )
		{
			glm::vec3 SubNewLoc = submarine.get_transform().GetTranslation();
			SubNewLoc.z += fMovingSpeed;
			submarine.get_transform().SetTranslate(SubNewLoc);
		}

		mWindowManager.NewImGuiFrame();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		bonobo::changePolygonMode(polygon_mode);

		if (!shader_reload_failed) {
			//
			// Todo: Render all your geometry here.
			//
			if (show_control_points){
				for (auto const& control_point : control_points) {
					control_point.render(mCamera.GetWorldToClipMatrix());
				}
			}
			
			skybox.render(mCamera.GetWorldToClipMatrix());

			tuna.render(mCamera.GetWorldToClipMatrix());
			tuna.get_transform().SetTranslate(glm::vec3(2.0f, 2.0f, 2.0f));

			submarine.render(mCamera.GetWorldToClipMatrix());

			shark.render(mCamera.GetWorldToClipMatrix());
			shark.get_transform().SetTranslate(glm::vec3(-2.0f, -2.0f, -2.0f));

			treasure.render(mCamera.GetWorldToClipMatrix());
			treasure.get_transform().SetTranslate(glm::vec3(0.0f, -5.0f, 0.0f));
			
			
			edaf80::Assignment5::moveObject(tuna, control_point_locations, 5.0f, elapsed_time_s, CIRCULAR);

			auto const test_set_uniforms = [&light_position, &elapsed_time_s](GLuint program) {
				glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
				glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
				};
		}


		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//
		bool const opened = ImGui::Begin("Scene Controls", nullptr, ImGuiWindowFlags_None);
		if (opened) {
			ImGui::Checkbox("Show basis", &show_basis);
			ImGui::Checkbox("Use normal mapping", &use_normal_mapping);
			ImGui::SliderFloat("Basis thickness scale", &basis_thickness_scale, 0.0f, 100.0f);
			ImGui::SliderFloat("Basis length scale", &basis_length_scale, 0.0f, 100.0f);
			auto const cull_mode_changed = bonobo::uiSelectCullMode("Cull mode", cull_mode);
			if (cull_mode_changed) {
				changeCullMode(cull_mode);
			}
			bonobo::uiSelectPolygonMode("Polygon mode", polygon_mode);
			ImGui::Checkbox("Pause animation", &pause_animation);
			ImGui::Checkbox("Enable interpolation", &interpolate);
			ImGui::Checkbox("Show control points", &show_control_points);
			bonobo::uiSelectPolygonMode("Polygon mode", polygon_mode);
			ImGui::Separator();
			ImGui::SliderFloat3("Light Position", glm::value_ptr(light_position), -20.0f, 20.0f);
		}
		ImGui::End();

		if (show_basis)
			bonobo::renderBasis(basis_thickness_scale, basis_length_scale, mCamera.GetWorldToClipMatrix());
		if (show_logs)
			Log::View::Render();
		mWindowManager.RenderImGuiFrame(show_gui);

		glfwSwapBuffers(window);
	}
}

int main()
{
	std::setlocale(LC_ALL, "");

	Bonobo framework;

	try {
		edaf80::Assignment5 assignment5(framework.GetWindowManager());
		assignment5.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}

void edaf80::Assignment5::moveObject(Node& Object, const std::vector<glm::vec3>& _control_point_locations,
									float duration_s, float elapsed_time_s, MovingStyle movingStyle)
{
	float pi = 3.14f;
	
	
	glm::vec3 newLoc;
	int i = 0;

	if (CONTROL_POINTS == movingStyle)
	{
		int numb_cp = _control_point_locations.size();
		float x = fmod(elapsed_time_s, duration_s);
		int index = static_cast<int> (elapsed_time_s / duration_s);
		float catmull_rom_tension = 0.5f;

		glm::vec3 p0 = _control_point_locations[index % numb_cp];
		glm::vec3 p1 = _control_point_locations[(index + 1) % numb_cp];
		glm::vec3 p2 = _control_point_locations[(index + 2) % numb_cp];
		glm::vec3 p3 = _control_point_locations[(index + 3) % numb_cp];
		newLoc = interpolation::evalCatmullRom(p0, p1, p2, p3, catmull_rom_tension, x);
		Object.get_transform().SetTranslate(newLoc);
	}

	else if (CIRCULAR == movingStyle)
	{
		float movingSpd = pi / 10;
		glm::vec3 ObjectLoc = Object.get_transform().GetTranslation();
		float vecLen = sqrt(pow(ObjectLoc.x, 2) + pow(ObjectLoc.y, 2) + pow(ObjectLoc.z, 2));
		newLoc = glm::vec3(vecLen * cos(movingSpd * elapsed_time_s), ObjectLoc.y, vecLen * sin(movingSpd * elapsed_time_s));
		Object.get_transform().SetTranslate(newLoc);

		glm::vec3 DirectionVec = normalize(newLoc - ObjectLoc);
		Object.get_transform().LookTowards(DirectionVec);
	}
	
}
