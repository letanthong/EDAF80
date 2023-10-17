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

	GLuint diffuse_shader = 0u;
	program_manager.CreateAndRegisterProgram("Diffuse",
		{ { ShaderType::vertex, "EDAF80/diffuse.vert" },
		  { ShaderType::fragment, "EDAF80/diffuse.frag" } },
		diffuse_shader);
	if (diffuse_shader == 0u)
		LogError("Failed to load diffuse shader");

	GLuint skybox_shader = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
		{ { ShaderType::vertex, "EDAF80/skybox.vert" },
		  { ShaderType::fragment, "EDAF80/skybox.frag" } },
		skybox_shader
	);
	if (skybox_shader == 0u)
		LogError("Failed to load skybox shader");

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
	if (shark_shader == 0u)
		LogError("Failed to load shark shader");
	

	auto light_position = glm::vec3(-2.0f, 4.0f, 2.0f);
	auto const set_uniforms = [&light_position](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position)); };

	//Skybox
	auto skybox_shape = parametric_shapes::createSphere(100.0f, 1000u, 1000u);
	if (skybox_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}

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

	//Test point shape
	auto test_point = parametric_shapes::createSphere(1.0f, 10u, 10u);
	if (test_point.vao == 0u) {
		LogError("Failed to retrieve the mesh for the test point");
		return;
	}
	Node test;
	test.set_geometry(test_point);
	//End test

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

	std::vector <glm::vec3> tuna_locations = {
	glm::vec3(10.0f,  10.0f,  15.0f),
	glm::vec3(20.0f,  25.0f,  10.0f),
	glm::vec3(22.0f,  12.0f,  10.0f),
	glm::vec3(20.0f,  15.0f,  20.0f),
	glm::vec3(25.0f,  10.0f,  15.0f),
	glm::vec3(-20.0f, -18.0f,  18.0f),
	glm::vec3(-25.0f, -15.0f, -20.0f),
	glm::vec3(-20.0f, -28.0f, -20.0f),
	glm::vec3(-23.0f, -18.0f, -24.0f)
	};

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
	shark.get_transform().SetScale(0.3f);
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

	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);


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

	int iRotationCnt = 0;
	int iCollisionCnt = 0;
	float const fSharkCollRadius = 1.0f;
	float const fTunaCollRadius = 1.0f;
	float const fCoinCollRadius = 1.0f;
	float const fSubMarineCollRadius = 1.0f;
	float const fTreasureCollRadius = 1.0f;
	bool circularMovementSet = false;

	float elapsed_time_s = 0.0f;
	float delta_time_s = 0.0f;
	bool pause_animation = false;

	auto const coin_set_uniforms =
		[&light_position, &camera_position, &coin_material, &elapsed_time_s](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform3fv(glGetUniformLocation(program, "ambient_colour"), 1, glm::value_ptr(coin_material.ambient));
		glUniform3fv(glGetUniformLocation(program, "diffuse_colour"), 1, glm::value_ptr(coin_material.diffuse));
		glUniform3fv(glGetUniformLocation(program, "specular_colour"), 1, glm::value_ptr(coin_material.specular));
		glUniform1f(glGetUniformLocation(program, "shininess"), coin_material.shininess);
		glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
		};

	std::vector <glm::vec3> coin_locations = {
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

	std::array<Node, 9> coins;
	for (std::size_t i = 0; i < coin_locations.size(); ++i) {
		auto& coin = coins[i];
		coin.set_geometry(coin_model.at(0));
		coin.get_transform().SetTranslate(coin_locations[i]);
		coin.set_material_constants(coin_material);
		coin.add_texture("coin_diff", coin_diff, GL_TEXTURE_2D);
		coin.add_texture("coin_spec", coin_rough, GL_TEXTURE_2D);
		coin.add_texture("coin_normal", coin_normal, GL_TEXTURE_2D);
		coin.set_program(&shark_shader, coin_set_uniforms);
	}
	//End control points
	GameState gameState = PLAY;

	while (!glfwWindowShouldClose(window)) {
		float fMovingSpeed = 0.1f;
		float Pi = 3.14f;
		float fMovingRotAngle = Pi / 8;

		auto& io = ImGui::GetIO();
		auto const nowTime = std::chrono::high_resolution_clock::now();
		auto const deltaTimeUs = std::chrono::duration_cast<std::chrono::microseconds>(nowTime - lastTime);
		lastTime = nowTime;
		if (!pause_animation) {
			delta_time_s = std::chrono::duration<float>(deltaTimeUs).count();
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

		switch (gameState)
		{
		case MENU:
			break;

		case PLAY:
			inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);
			glfwPollEvents();
			inputHandler.Advance();
			mCamera.Update(deltaTimeUs, inputHandler, true, true);

			glm::vec3 SubmarineLoc = submarine.get_transform().GetTranslation();
			glm::vec3 SubLocOffset = glm::vec3(0.0f, 3.0f, 5.0f);
			light_position = SubmarineLoc + SubLocOffset;
			mCamera.mWorld.SetTranslate(SubmarineLoc + SubLocOffset);
			mCamera.mWorld.LookAt(SubmarineLoc - SubLocOffset);
			
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

			int framebuffer_width, framebuffer_height;
			glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
			glViewport(0, 0, framebuffer_width, framebuffer_height);

			//Left turn
			if ((inputHandler.GetKeycodeState(GLFW_KEY_LEFT) & JUST_PRESSED) || (inputHandler.GetKeycodeState(GLFW_KEY_LEFT) & PRESSED))
			{

				glm::vec3 SubNewLoc = submarine.get_transform().GetTranslation();
				SubNewLoc.x -= fMovingSpeed;
				submarine.get_transform().SetTranslate(SubNewLoc);
				if (iRotationCnt == 0)
				{
					submarine.get_transform().RotateZ(-fMovingRotAngle);
					++iRotationCnt;
				}

			}
			if ((inputHandler.GetKeycodeState(GLFW_KEY_LEFT) & JUST_RELEASED))
			{
				submarine.get_transform().RotateZ(fMovingRotAngle);
				iRotationCnt = 0;
			}

			//Right turn
			if ((inputHandler.GetKeycodeState(GLFW_KEY_RIGHT) & JUST_PRESSED) || (inputHandler.GetKeycodeState(GLFW_KEY_RIGHT) & PRESSED))
			{
				glm::vec3 SubNewLoc = submarine.get_transform().GetTranslation();
				SubNewLoc.x += fMovingSpeed;
				submarine.get_transform().SetTranslate(SubNewLoc);
				if (iRotationCnt == 0)
				{
					submarine.get_transform().RotateZ(fMovingRotAngle);
					++iRotationCnt;
				}
			}
			if ( (inputHandler.GetKeycodeState(GLFW_KEY_RIGHT) & JUST_RELEASED))
			{
				submarine.get_transform().RotateZ(-fMovingRotAngle);
				iRotationCnt = 0;
			}

			//Up
			if ((inputHandler.GetKeycodeState(GLFW_KEY_UP) & JUST_PRESSED || inputHandler.GetKeycodeState(GLFW_KEY_UP) & PRESSED))
			{
				glm::vec3 SubNewLoc = submarine.get_transform().GetTranslation();
				SubNewLoc.y += fMovingSpeed;
				submarine.get_transform().SetTranslate(SubNewLoc);
			}

			//Down
			if ((inputHandler.GetKeycodeState(GLFW_KEY_DOWN) & JUST_PRESSED || inputHandler.GetKeycodeState(GLFW_KEY_DOWN) & PRESSED))
			{
				glm::vec3 SubNewLoc = submarine.get_transform().GetTranslation();
				SubNewLoc.y -= fMovingSpeed;
				submarine.get_transform().SetTranslate(SubNewLoc);
			}

			//forward
			if ((inputHandler.GetKeycodeState(GLFW_KEY_W) & JUST_PRESSED || inputHandler.GetKeycodeState(GLFW_KEY_W) & PRESSED))
			{
				glm::vec3 SubNewLoc = submarine.get_transform().GetTranslation();
				SubNewLoc.z -= fMovingSpeed;
				submarine.get_transform().SetTranslate(SubNewLoc);
			}

			//backward
			if ((inputHandler.GetKeycodeState(GLFW_KEY_S) & JUST_PRESSED || inputHandler.GetKeycodeState(GLFW_KEY_S) & PRESSED))
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
				if (show_control_points) {
					for (auto const& _coin : coins) {
						_coin.render(mCamera.GetWorldToClipMatrix());
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

				edaf80::Assignment5::moveObjectCircular(tuna, Pi/10 , CLOCKWISE , elapsed_time_s);
				//edaf80::Assignment5::moveObjectLinear(tuna, 0.1f, glm::vec3(1.0f, 0.0f, 1.0f), elapsed_time_s);
				edaf80::Assignment5::moveObjectCircular(shark, Pi / 10, CLOCKWISE, elapsed_time_s);
				iCollisionCnt += edaf80::Assignment5::collisionCount(submarine, tuna, fSharkCollRadius, fTunaCollRadius);

			}

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			break;

		case WIN:
			break;

		case LOSE:
			break;

		case EXIT:
			break;
		}

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

void edaf80::Assignment5::moveObjectCircular(Node& Object, float Omega, enum CircularMovement direction, float elapsed_time_s)
{
	float pi = 3.14f;
	glm::vec3 newLoc;

	glm::vec3 ObjectLoc = Object.get_transform().GetTranslation();
	float vecLen = sqrt( pow(ObjectLoc.x,2) + pow(ObjectLoc.y, 2) + pow(ObjectLoc.z, 2) );
	newLoc = glm::vec3(vecLen * cos(Omega * elapsed_time_s), ObjectLoc.y, vecLen * sin(Omega * elapsed_time_s));
	Object.get_transform().LookAt(newLoc);
	Object.get_transform().RotateY((pi + Omega) / 2);
	Object.get_transform().SetTranslate(newLoc);
	
}

void edaf80::Assignment5::moveObjectLinear(Node& Object, float movingSpeed, glm::vec3 MovingDirection, float elapsed_time_s)
{
	float pi = 3.14f;
	glm::vec3 newLoc;

	glm::vec3 ObjectLoc = Object.get_transform().GetTranslation();
	newLoc = ObjectLoc + movingSpeed * elapsed_time_s * normalize(MovingDirection);
	Object.get_transform().LookAt(newLoc);
	Object.get_transform().RotateY(pi);
	Object.get_transform().SetTranslate(newLoc);
}

int edaf80::Assignment5::collisionCount(Node& Object1, Node& Object2, float ObjCollRadius1, float ObjCollRadius2)
{
	int iCollCnt = 0;
	glm::vec3 ObjLoc1 = Object1.get_transform().GetTranslation();
	glm::vec3 ObjLoc2 = Object2.get_transform().GetTranslation();

	glm::vec3 distVec = ObjLoc2 - ObjLoc1;
	float collDist = sqrt(pow(distVec.x, 2) + pow(distVec.y, 2) + pow(distVec.z, 2) );
	if (collDist < (ObjCollRadius1 + ObjCollRadius2))
	{
		++iCollCnt;
	}
	return iCollCnt;
}

int main()
{
	std::setlocale(LC_ALL, "");

	Bonobo framework;

	try {
		edaf80::Assignment5 assignment5(framework.GetWindowManager());
		assignment5.run();
	}
	catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}
