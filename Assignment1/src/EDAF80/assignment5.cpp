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
#include <stdlib.h>

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

	//Register diffuse shader
	GLuint diffuse_shader = 0u;
	program_manager.CreateAndRegisterProgram("Diffuse",
		{ { ShaderType::vertex, "EDAF80/diffuse.vert" },
		  { ShaderType::fragment, "EDAF80/diffuse.frag" } },
		diffuse_shader);
	if (diffuse_shader == 0u)
		LogError("Failed to load diffuse shader");

	//Register skybox shader
	GLuint skybox_shader = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
		{ { ShaderType::vertex, "EDAF80/skybox.vert" },
		  { ShaderType::fragment, "EDAF80/skybox.frag" } },
		skybox_shader
	);
	if (skybox_shader == 0u)
		LogError("Failed to load skybox shader");

	//Register tuna shader
	GLuint tuna_shader = 0u;
	program_manager.CreateAndRegisterProgram("Tuna",
		{ { ShaderType::vertex, "EDAF80/tuna.vert" },
		  { ShaderType::fragment, "EDAF80/tuna.frag" } },
		tuna_shader
	);
	if (tuna_shader == 0u)
		LogError("Failed to load tuna shader");

	//Register shark shader
	GLuint shark_shader = 0u;
	program_manager.CreateAndRegisterProgram("Shark",
		{ { ShaderType::vertex, "EDAF80/shark.vert" },
		  { ShaderType::fragment, "EDAF80/shark.frag" } },
		shark_shader
	);
	if (shark_shader == 0u)
		LogError("Failed to load shark shader");

	//Register bubble shader
	GLuint bubble_shader = 0u;
	program_manager.CreateAndRegisterProgram("Bubble",
		{ { ShaderType::vertex, "EDAF80/bubble.vert" },
		  { ShaderType::fragment, "EDAF80/bubble.frag" } },
		bubble_shader
	);
	if (bubble_shader == 0u)
		LogError("Failed to load bubble shader");

	auto light_position = glm::vec3(2.0f, 4.0f, 2.0f);
	auto light_position_2 = glm::vec3(-2.0f, 4.0f, 2.0f);
	auto const set_uniforms = [&light_position, &light_position_2](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "light_position_2"), 1, glm::value_ptr(light_position_2));
		};

	//Skybox
	auto skybox_shape = parametric_shapes::createSphere(15.0f, 100u, 100u);
	if (skybox_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}

	//Bubble shape
	auto bubble_shape = parametric_shapes::createSphere(0.1f, 30u, 30u);
	if (bubble_shape.vao == 0u) {
		LogError("Failed to retrieve the mesh for the skybox");
		return;
	}

	//Skybox load textures
	GLuint const skybox_texture = bonobo::loadTextureCubeMap(
		config::resources_path("cubemaps/Underwater/uw_ft_posx.jpg"),
		config::resources_path("cubemaps/Underwater/uw_bk_negx.jpg"),
		config::resources_path("cubemaps/Underwater/uw_up_posy.jpg"),
		config::resources_path("cubemaps/Underwater/uw_dn_negy.jpg"),
		config::resources_path("cubemaps/Underwater/uw_rt_posz.jpg"),
		config::resources_path("cubemaps/Underwater/uw_lf_negz.jpg"),
		true
	);

	//Skybox declaration
	Node skybox;
	skybox.set_geometry(skybox_shape);
	skybox.add_texture("skybox_texture", skybox_texture, GL_TEXTURE_CUBE_MAP);
	skybox.set_program(&skybox_shader, set_uniforms);


	//Tuna material
	bonobo::material_data tuna_material;
	tuna_material.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	tuna_material.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
	tuna_material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	tuna_material.shininess = 5.0f;

	//Tuna load models
	std::vector<bonobo::mesh_data> tuna_model;
	tuna_model = bonobo::loadObjects(config::resources_path("textures/tuna/tuna_fish.obj"));
	GLuint const tuna_body_diff = bonobo::loadTexture2D(config::resources_path("textures/tuna/tuna_body_diff.png"), true);
	GLuint const tuna_body_rough = bonobo::loadTexture2D(config::resources_path("textures/tuna/tuna_body_rough.png"), true);
	GLuint const tuna_body_normal = bonobo::loadTexture2D(config::resources_path("textures/tuna/tuna_body_normal.png"), true);

	//Shark material
	bonobo::material_data shark_material;
	shark_material.ambient = glm::vec3(0.5f, 0.5f, 0.5f);
	shark_material.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
	shark_material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	shark_material.shininess = 10.0f;

	//Shark load model
	std::vector<bonobo::mesh_data> shark_model;
	shark_model = bonobo::loadObjects(config::resources_path("textures/shark/hammershark.obj"));
	GLuint const shark_diff = bonobo::loadTexture2D(config::resources_path("textures/shark/hammershark_diff.png"), true);
	GLuint const shark_rough = bonobo::loadTexture2D(config::resources_path("textures/shark/hammershark_spec.png"), true);
	GLuint const shark_normal = bonobo::loadTexture2D(config::resources_path("textures/shark/hammershark_normal.png"), true);

	//Seasweed material
	bonobo::material_data seaweed_material;
	seaweed_material.ambient = glm::vec3(0.3f, 0.3f, 0.3f);
	seaweed_material.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
	seaweed_material.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	seaweed_material.shininess = 10.0f;

	//Bubble material
	bonobo::material_data bubble_material;
	bubble_material.ambient = glm::vec3(0.0f, 0.2f, 0.7f);
	bubble_material.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	bubble_material.specular = glm::vec3(0.2f, 0.5f, 0.5f);
	bubble_material.shininess = 10.0f;

	//Seaweed load model
	std::vector<bonobo::mesh_data> seaweed_model;
	seaweed_model = bonobo::loadObjects(config::resources_path("textures/seaweed/tall_seaweed.obj"));
	GLuint const seaweed_diff = bonobo::loadTexture2D(config::resources_path("textures/seaweed/seaweed_diff.png"), true);
	GLuint const seaweed_spec = bonobo::loadTexture2D(config::resources_path("textures/seaweed/seaweed_spec.png"), true);
	GLuint const seaweed_normal = bonobo::loadTexture2D(config::resources_path("textures/seaweed/seaweed_normal.png"), true);
	//End seaweed
	

	glClearDepthf(1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	float Pi = 3.14f;
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

	float elapsed_time_s = 0.0f;
	float delta_time_s = 0.0f;
	bool pause_animation = false;


	//Objects vectors
	std::vector<Node> tunas;
	std::vector<Node> sharks;
	std::vector<Node> seaweeds;
	std::vector<Node> bubbles;
	int iMaxNumberofTunas = 3;
	int iMaxNumberofSharks = 0;
	int iMaxNumberofSeaweeds = 0;
	int iMaxNumberofBubbles = 5;
	const int iGameRadius = 5;

	//Setup seaweed uniform
	auto const seaweed_set_uniforms =
		[&light_position, &camera_position, &seaweed_material, &elapsed_time_s](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform3fv(glGetUniformLocation(program, "ambient_colour"), 1, glm::value_ptr(seaweed_material.ambient));
		glUniform3fv(glGetUniformLocation(program, "diffuse_colour"), 1, glm::value_ptr(seaweed_material.diffuse));
		glUniform3fv(glGetUniformLocation(program, "specular_colour"), 1, glm::value_ptr(seaweed_material.specular));
		glUniform1f(glGetUniformLocation(program, "shininess"), seaweed_material.shininess);
		glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
		};

	//Setup bubble uniform
	auto const bubble_set_uniforms =
		[&light_position, &camera_position, &bubble_material](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform3fv(glGetUniformLocation(program, "ambient_colour"), 1, glm::value_ptr(bubble_material.ambient));
		glUniform3fv(glGetUniformLocation(program, "diffuse_colour"), 1, glm::value_ptr(bubble_material.diffuse));
		glUniform3fv(glGetUniformLocation(program, "specular_colour"), 1, glm::value_ptr(bubble_material.specular));
		glUniform1f(glGetUniformLocation(program, "shininess"), bubble_material.shininess);
		};

	//Initialize position of seaweeds
	for (std::size_t i = 0; i < iMaxNumberofSeaweeds; ++i) {
		Node _seaweed;
		glm::vec3 seaweed_location = glm::vec3((rand() % iGameRadius), (rand() % iGameRadius), (rand() % iGameRadius)); //Random locations of coins
		_seaweed.set_geometry(seaweed_model.at(0));
		_seaweed.get_transform().SetTranslate(seaweed_location);
		_seaweed.set_material_constants(seaweed_material);
		_seaweed.add_texture("seaweed_diff", seaweed_diff, GL_TEXTURE_2D);
		_seaweed.add_texture("seaweed_spec", seaweed_spec, GL_TEXTURE_2D);
		_seaweed.add_texture("seaweed_normal", seaweed_normal, GL_TEXTURE_2D);
		_seaweed.get_transform().SetScale(rand()%10);
		_seaweed.set_program(&shark_shader, seaweed_set_uniforms);
		seaweeds.push_back(_seaweed);
	}

	//Initialize position of bubbles
	for (std::size_t i = 0; i < iMaxNumberofBubbles; ++i) {
		Node _bubble;
		glm::vec3 bubble_location = glm::vec3((rand() % iGameRadius), (rand() % iGameRadius), (rand() % iGameRadius)); //Random locations of coins
		_bubble.set_geometry(bubble_shape);
		_bubble.get_transform().SetTranslate(bubble_location);
		_bubble.get_transform().SetScale(rand()%5);
		_bubble.set_program(&bubble_shader, bubble_set_uniforms);
		bubbles.push_back(_bubble);
	}

	//Initialize position of tunas
	for (std::size_t i = 0; i < iMaxNumberofTunas; ++i) {
		Node _tuna;
		glm::vec3 tuna_location = glm::vec3((rand() % iGameRadius), (rand() % iGameRadius), (rand() % iGameRadius)); //Random locations of fishes
		_tuna.set_geometry(tuna_model.at(0));
		_tuna.get_transform().SetTranslate(tuna_location);
		_tuna.add_texture("tuna_body_diff", tuna_body_diff, GL_TEXTURE_2D);
		_tuna.add_texture("tuna_body_rough", tuna_body_rough, GL_TEXTURE_2D);
		_tuna.add_texture("tuna_body_normal", tuna_body_normal, GL_TEXTURE_2D);
		_tuna.set_material_constants(tuna_material);
		tunas.push_back(_tuna);
	}

	//Initialize tuna moving radius
	std::vector<float> fTunaMovingRadius;
	for (std::size_t i = 0; i < tunas.size(); i++)
	{
		fTunaMovingRadius.push_back(rand() % iGameRadius);
	}

	//Initialize tuna moving omega
	std::vector<float> CircularMovingSpeed;
	for (std::size_t i = 0; i < tunas.size(); i++)
	{
		CircularMovingSpeed.push_back((rand() % 4)*(Pi/20));
	}

	//Initialize linear moving direction
	std::vector<glm::vec3> LinearMovingDirection;
	for (std::size_t i = 0; i < tunas.size(); i++)
	{
		LinearMovingDirection.push_back(glm::vec3((rand() % iGameRadius), (rand() % iGameRadius), (rand() % iGameRadius)));
	}

	//Initial position of sharks
	for (std::size_t i = 0; i < iMaxNumberofSharks; ++i) {
		Node _shark;
		glm::vec3 shark_location = glm::vec3((rand() % iGameRadius), (rand() % iGameRadius), (rand() % iGameRadius)); //Random locations of fishes
		_shark.set_geometry(shark_model.at(0));
		_shark.add_texture("shark_diff", shark_diff, GL_TEXTURE_2D);
		_shark.add_texture("shark_rough", shark_rough, GL_TEXTURE_2D);
		_shark.add_texture("shark_normal", shark_normal, GL_TEXTURE_2D);
		_shark.set_material_constants(shark_material);
		_shark.get_transform().SetScale(rand()%5);
		_shark.get_transform().SetTranslate(shark_location);
		sharks.push_back(_shark);
	}

	//Initailize shark moving radius
	std::vector<float> fSharkMovingRadius;
	for (std::size_t i = 0; i < sharks.size(); i++)
	{
		fSharkMovingRadius.push_back(rand() % iGameRadius);
	}

	//Initialize shark moving speed
	std::vector<float> fSharkMovingSpeed;
	for (std::size_t i = 0; i < sharks.size(); i++)
	{
		fSharkMovingSpeed.push_back((rand() % 5) * (Pi / 30));
	}

	while (!glfwWindowShouldClose(window)) {
		bool menuOpen = false;
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
			[&use_normal_mapping, &light_position, &light_position_2, &camera_position, &tuna_material, &elapsed_time_s](GLuint program) {
			glUniform1i(glGetUniformLocation(program, "use_normal_mapping"), use_normal_mapping ? 1 : 0);
			glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
			glUniform3fv(glGetUniformLocation(program, "light_position_2"), 1, glm::value_ptr(light_position_2));
			glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
			glUniform3fv(glGetUniformLocation(program, "ambient_colour"), 1, glm::value_ptr(tuna_material.ambient));
			glUniform3fv(glGetUniformLocation(program, "diffuse_colour"), 1, glm::value_ptr(tuna_material.diffuse));
			glUniform3fv(glGetUniformLocation(program, "specular_colour"), 1, glm::value_ptr(tuna_material.specular));
			glUniform1f(glGetUniformLocation(program, "shininess"), tuna_material.shininess);
			glUniform1f(glGetUniformLocation(program, "elapsed_time_s"), elapsed_time_s);
			};

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

		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);
		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(deltaTimeUs, inputHandler, false, false);
		int framebuffer_width, framebuffer_height;
		std::string sMessage = "";
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


		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);

		mWindowManager.NewImGuiFrame();
		menuOpen = ImGui::Begin("Control panel", nullptr, ImGuiWindowFlags_None);
		if (menuOpen) {
			ImGui::Checkbox("Pause animation", &pause_animation);
			bonobo::uiSelectPolygonMode("Polygon mode", polygon_mode);
		}
		ImGui::End();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		bonobo::changePolygonMode(polygon_mode);

		if (!shader_reload_failed) {

			skybox.render(mCamera.GetWorldToClipMatrix());

			//Render seaweeds
			for (int i = 0; i < seaweeds.size(); i++)
			{
				seaweeds.at(i).render(mCamera.GetWorldToClipMatrix());
				seaweeds.at(i).get_transform().RotateY(Pi / 200);
			}

			//Render bubbles
			for (int i = 0; i < bubbles.size(); i++)
			{
				bubbles.at(i).render(mCamera.GetWorldToClipMatrix());
				//bubble moving up
				// Temporarily remove bubbles movement
				//edaf80::Assignment5::moveObjectLinear(bubbles.at(i), 0.004f, glm::vec3(0.0f, 1.0f, 0.0f), elapsed_time_s);
				glm::vec3 bubbleLoc = bubbles.at(i).get_transform().GetTranslation();
				//Relocate to new location if moving out of the gamezone
				if (bubbleLoc.x > iGameRadius || bubbleLoc.x < -iGameRadius ||
					bubbleLoc.y > iGameRadius || bubbleLoc.y < -iGameRadius ||
					bubbleLoc.z > iGameRadius || bubbleLoc.z < -iGameRadius)
				{
					bubbles.at(i).get_transform().SetTranslate(glm::vec3((rand() % iGameRadius), (rand() % iGameRadius), (rand() % iGameRadius)));
				}
			}

			//Render tunas
			/*for (int i = 0; i < tunas.size(); i++)
			{
				tunas.at(i).set_program(&tuna_shader, tuna_set_uniforms);
				tunas.at(i).render(mCamera.GetWorldToClipMatrix());
				edaf80::Assignment5::moveObjectCircular(tunas.at(i), CircularMovingSpeed.at(i), fTunaMovingRadius.at(i), CLOCKWISE, elapsed_time_s);
			}*/

			//change location of the first tuna back to the origin
			tunas.at(0).get_transform().SetTranslate(glm::vec3(0.0f));
			tunas.at(0).set_program(&tuna_shader, tuna_set_uniforms);
			tunas.at(0).render(mCamera.GetWorldToClipMatrix());
			//Render sharks
			for (int i = 0; i < sharks.size(); i++)
			{
				sharks.at(i).set_program(&tuna_shader, tuna_set_uniforms);
				sharks.at(i).render(mCamera.GetWorldToClipMatrix());
				edaf80::Assignment5::moveObjectCircular(sharks.at(i), fSharkMovingSpeed.at(i), fSharkMovingRadius.at(i), CLOCKWISE, elapsed_time_s);
			}

			/*sharks.at(0).get_transform().SetTranslate(glm::vec3(2.0f));
			sharks.at(0).set_program(&shark_shader, shark_set_uniforms);
			sharks.at(0).render(mCamera.GetWorldToClipMatrix());*/
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (show_basis)
			bonobo::renderBasis(basis_thickness_scale, basis_length_scale, mCamera.GetWorldToClipMatrix());
		if (show_logs)
			Log::View::Render();
		mWindowManager.RenderImGuiFrame(show_gui);

		glfwSwapBuffers(window);
	}
}

void edaf80::Assignment5::moveObjectCircular(Node& Object, float Omega, float Radius, enum CircularMovement direction, float elapsed_time_s)
{
	float pi = 3.14f;
	glm::vec3 newLoc;

	glm::vec3 ObjectLoc = Object.get_transform().GetTranslation();
	//float vecLen = sqrt( pow(ObjectLoc.x,2) + pow(ObjectLoc.y, 2) + pow(ObjectLoc.z, 2) );
	float vecLen = abs(Radius);
	if (vecLen < 10)
	{
		vecLen = 10; //default length
	}
	newLoc = glm::vec3(vecLen * cos(Omega * elapsed_time_s), ObjectLoc.y, vecLen * sin(Omega * elapsed_time_s));
	Object.get_transform().LookAt(newLoc);
	Object.get_transform().RotateY(pi);
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
