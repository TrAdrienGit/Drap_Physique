#define STB_IMAGE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <list>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <image_loader/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#include <chrono>
#include <thread>
#include <omp.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Drap_Physique.h"
#include "glencapsulation.h"
#include "config.h"
#include "light.h"
#include "shader.h"
#include "simple_camera.h"
#include "mesh_generator.h"
#include "physicSoA.h"
#include "tissuSoA.h"
#include "grabbing.h"
#include "command_console.h"
#include "export_obj_frame.h"

// -------------------------------------------------------------------------------------------

std::string configPath = "json/config.json";


// -------------------------------------------------------------------------------------------
// Time counter
float deltaTime = 0.0f;
float lastFrame = 0.0f;
int frameCount = 0;
double fpsTimeAccumulator = 0.0;

// -------------------------------------------------------------------------------------------
// Fonction Declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// -------------------------------------------------------------------------------------------
// Main
int main()
{
	// -------------------------------------------------------------------------------------------
	// Cap Thread Number
	omp_set_num_threads(8);

	// -------------------------------------------------------------------------------------------
	// Load config
	Config::loadFromJSON(configPath);

	// -------------------------------------------------------------------------------------------
	// Init
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	GLFWwindow* window = glfwCreateWindow(screenSettings.width, screenSettings.height, "Tissu_Physique", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // V-sync
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	// -------------------------------------------------------------------------------------------
	// Initialisation Shader
	Shader simulationShader("shaders/main_vertex_shader.vs", "shaders/main_fragment_shader.fs");

	// -------------------------------------------------------------------------------------------
	// Initialisation Tissu
	Tissu::TissuSoA LeTissu = Tissu::TissuSoA(tissuSettings);
	//Fixation de certain points
	LeTissu.lockCorner(lockCorner);

	if (debug)
	{
		LeTissu.printVertexPosition();
		LeTissu.printVertexNormal();
		LeTissu.printVertexUV();
		LeTissu.printVBOBuffer();
		LeTissu.printEBOBuffer();
	}

	// -------------------------------------------------------------
	// Initialisation Renderable 
	auto tissu_diffuse_texture = std::make_shared<GL::Texture>("textures/drap/FabricPlainNaturalSheer009COL2K.jpg", GL::TextureFormat::JPG);
	auto tissu_specular_texture = std::make_shared<GL::Texture>("textures/generic/black.png", GL::TextureFormat::PNG);
	auto tissu_emissive_texture = std::make_shared<GL::Texture>("textures/generic/black.png", GL::TextureFormat::PNG);
	GL::Material tissu_material = GL::Material(tissu_diffuse_texture, tissu_specular_texture, tissu_emissive_texture, 64.0f);
	auto tissu_mesh = std::make_shared <GL::Mesh>(LeTissu.VBOBuffer, LeTissu.EBOBuffer);
	GL::Model tissu_model = GL::Model(tissu_mesh);
	GL::Renderable tissuRenderable = GL::Renderable(std::move(tissu_model), std::move(tissu_material), simulationShader, glm::vec2(screenSettings.width, screenSettings.height), false);
	tissuRenderable.model.mesh->sendEBOBuffer();

	// -------------------------------------------------------------
	// Model Matrix (Modifie l'objet) Translate/Rotate/Scale
	// Centering plane
	glm::mat4 tissuModelMatrix = glm::mat4(1.0f);
	tissuRenderable.modelMatrix = tissuModelMatrix;
	tissuRenderable.projectionMatrix = SimpleCamera::computeProjectionMatrix(screenSettings);
	
	// -------------------------------------------------------------
	// Lighting
	Light::sendToShader(lightSettings, tissuRenderable.shader);

	// -------------------------------------------------------------
	// Model Loading
	std::vector<MeshGenerator::MeshGenerator*> listeObjectCollisionable;

	// Sphere
	MeshGenerator::MeshSettings sphere_settings;
	sphere_settings.position = glm::vec3(((LeTissu.tissuSettings.resolutionX - 1) * LeTissu.tissuSettings.gapPointStartX) / 2, ((LeTissu.tissuSettings.resolutionY - 1) * LeTissu.tissuSettings.gapPointStartY) / 2, -2.5f);
	sphere_settings.position += glm::vec3(1.7f, 0.0f, 0.0f);
	sphere_settings.radius = 2.7f;
	sphere_settings.resolutionX = 20;
	sphere_settings.resolutionY = 20;
	sphere_settings.rotationAxis = glm::vec3(1.0f, 1.0f, 1.0f);

	std::unique_ptr<MeshGenerator::MeshGenerator> sphereMeshGen = MeshGenerator::MeshFactory::createMesh(MeshGenerator::PrimitiveType::SPHERE, sphere_settings);
	if (debug)
	{
		sphereMeshGen->printVertexPosition();
		sphereMeshGen->printVertexNormal();
		sphereMeshGen->printVertexUV();
		sphereMeshGen->printVBOBuffer();
		sphereMeshGen->printEBOBuffer();
	}

	auto sphere_diffuse_texture = std::make_shared<GL::Texture>("textures/sphere/sphere_wood.jpg", GL::TextureFormat::JPG);
	auto sphere_specular_texture = std::make_shared<GL::Texture>("textures/generic/black.png", GL::TextureFormat::PNG);
	auto sphere_emissive_texture = std::make_shared<GL::Texture>("textures/generic/black.png", GL::TextureFormat::PNG);
	GL::Material sphere_material = GL::Material(sphere_diffuse_texture, sphere_specular_texture, sphere_emissive_texture, 64.0f);
	auto sphere_mesh = std::make_shared <GL::Mesh>(sphereMeshGen->VBOBuffer, sphereMeshGen->EBOBuffer);
	GL::Model sphere_model = GL::Model(sphere_mesh);
	GL::Renderable sphereRenderable = GL::Renderable(std::move(sphere_model), std::move(sphere_material), simulationShader);
	sphereRenderable.model.mesh->sendVBOBuffer();
	sphereRenderable.model.mesh->sendEBOBuffer();

	glm::mat4 sphereModelMatrix = glm::mat4(1.0f);
	sphereRenderable.modelMatrix = sphereModelMatrix;
	sphereRenderable.projectionMatrix = SimpleCamera::computeProjectionMatrix(screenSettings);

	listeObjectCollisionable.emplace_back(sphereMeshGen.get());


	// Box
	MeshGenerator::MeshSettings box_settings;
	box_settings.size = glm::vec3(2.0f, 4.0f, 2.0f);
	box_settings.position = glm::vec3(((LeTissu.tissuSettings.resolutionX - 1) * LeTissu.tissuSettings.gapPointStartX) / 2, ((LeTissu.tissuSettings.resolutionY - 1) * LeTissu.tissuSettings.gapPointStartY) / 2, -1.2f);
	box_settings.position += glm::vec3(-1.7f, 0.0f, 0.0f);
	box_settings.resolutionX = 4;
	box_settings.resolutionY = 4;
	box_settings.resolutionZ = 4;

	std::unique_ptr<MeshGenerator::MeshGenerator> boxMeshGen = MeshGenerator::MeshFactory::createMesh(MeshGenerator::PrimitiveType::BOX, box_settings);
	if (debug)
	{
		boxMeshGen->printVertexPosition();
		boxMeshGen->printVertexNormal();
		boxMeshGen->printVertexUV();
		boxMeshGen->printVBOBuffer();
		boxMeshGen->printEBOBuffer();
	}

	auto box_diffuse_texture = std::make_shared<GL::Texture>("textures/box/box_wood_2.png", GL::TextureFormat::PNG);
	auto box_specular_texture = std::make_shared<GL::Texture>("textures/generic/black.png", GL::TextureFormat::PNG);
	auto box_emissive_texture = std::make_shared<GL::Texture>("textures/generic/black.png", GL::TextureFormat::PNG);
	GL::Material box_material = GL::Material(box_diffuse_texture, box_specular_texture, box_emissive_texture, 64.0f);
	auto box_mesh = std::make_shared <GL::Mesh>(boxMeshGen->VBOBuffer, boxMeshGen->EBOBuffer);
	GL::Model box_model = GL::Model(box_mesh);
	GL::Renderable boxRenderable = GL::Renderable(std::move(box_model), std::move(box_material), simulationShader);
	boxRenderable.model.mesh->sendVBOBuffer();
	boxRenderable.model.mesh->sendEBOBuffer();
	
	glm::mat4 boxModelMatrix = glm::mat4(1.0f);
	boxRenderable.modelMatrix = boxModelMatrix;
	boxRenderable.projectionMatrix = SimpleCamera::computeProjectionMatrix(screenSettings);

	listeObjectCollisionable.emplace_back(boxMeshGen.get());


	//// Cylindre
	//MeshGenerator::MeshSettings cylinder_settings;
	//cylinder_settings.position = glm::vec3(((LeDrap.nb_point_x - 1) * LeDrap.gapPointInit) / 2, ((LeDrap.nb_point_y - 1) * LeDrap.gapPointInit) / 2, -2.5f);

	//std::unique_ptr<MeshGenerator::MeshGenerator> cylinderMeshGen = MeshGenerator::MeshFactory::createMesh(MeshGenerator::PrimitiveType::CYLINDER, cylinder_settings);

	//auto cylinder_diffuse_texture = std::make_shared<Texture>("textures/collision/Poliigon_WoodVeneerOak7760BaseColor.jpg", GL::TextureFormat::JPG);
	//auto cylinder_specular_texture = std::make_shared<Texture>("textures/generic/black.png", GL::TextureFormat::PNG);
	//auto cylinder_emissive_texture = std::make_shared<Texture>("textures/generic/black.png", GL::TextureFormat::PNG);
	//Material cylinder_material = Material(cylinder_diffuse_texture, cylinder_specular_texture, cylinder_emissive_texture, 64.0f);
	//auto cylinder_mesh = std::make_shared <Mesh>(cylinderMeshGen->VBOBuffer, cylinderMeshGen->EBOBuffer);
	//Model cylinder_model = Model(cylinder_mesh);
	//Renderable cylinderRenderable = Renderable(std::move(cylinder_model), std::move(cylinder_material), std::move(cylinderShader));
	//cylinderRenderable.model.mesh->sendVBOBuffer();
	//cylinderRenderable.model.mesh->sendEBOBuffer();

	//glm::mat4 cylinderModelMatrix = glm::mat4(1.0f);
	//cylinderRenderable.updateModelMatrix(cylinderModelMatrix);
	//cylinderRenderable.updateProjectionMatrix(computeProjectionMatrix(screenSettings.width, screenSettings.height));



	// cone (Light indicator)
	MeshGenerator::MeshSettings cone_settings;
	cone_settings.position = lightSettings.position;
	
	std::unique_ptr<MeshGenerator::MeshGenerator> coneMeshGen = MeshGenerator::MeshFactory::createMesh(MeshGenerator::PrimitiveType::CONE, cone_settings);

	auto cone_diffuse_texture = std::make_shared<GL::Texture>("textures/generic/blank.png", GL::TextureFormat::PNG);
	auto cone_specular_texture = std::make_shared<GL::Texture>("textures/generic/black.png", GL::TextureFormat::PNG);
	auto cone_emissive_texture = std::make_shared<GL::Texture>("textures/generic/black.png", GL::TextureFormat::PNG);
	GL::Material cone_material = GL::Material(cone_diffuse_texture, cone_specular_texture, cone_emissive_texture, 64.0f);
	auto cone_mesh = std::make_shared <GL::Mesh>(coneMeshGen->VBOBuffer, coneMeshGen->EBOBuffer);
	GL::Model cone_model = GL::Model(cone_mesh);
	GL::Renderable coneRenderable = GL::Renderable(std::move(cone_model), std::move(cone_material), simulationShader);
	coneRenderable.model.mesh->sendVBOBuffer();
	coneRenderable.model.mesh->sendEBOBuffer();

	glm::mat4 coneModelMatrix = glm::mat4(1.0f);
	coneRenderable.modelMatrix = coneModelMatrix;
	coneRenderable.projectionMatrix = SimpleCamera::computeProjectionMatrix(screenSettings);

	//listeObjectCollisionable.emplace_back(coneMeshGen.get());


	// disk (floor indicator)
	MeshGenerator::MeshSettings disk_settings;
	disk_settings.position = glm::vec3(((LeTissu.tissuSettings.resolutionX - 1) * LeTissu.tissuSettings.gapPointStartX) / 2, ((LeTissu.tissuSettings.resolutionY - 1) * LeTissu.tissuSettings.gapPointStartY) / 2, LeTissu.tissuSettings.floorHeight - 1.0f);
	disk_settings.radius = 10.0f;
	disk_settings.resolutionX = 50;

	std::unique_ptr<MeshGenerator::MeshGenerator> diskMeshGen = MeshGenerator::MeshFactory::createMesh(MeshGenerator::PrimitiveType::DISK, disk_settings);

	auto disk_diffuse_texture = std::make_shared<GL::Texture>("textures/floor/carpet_Base_Color.jpg", GL::TextureFormat::JPG);
	auto disk_specular_texture = std::make_shared<GL::Texture>("textures/generic/black.png", GL::TextureFormat::PNG);
	auto disk_emissive_texture = std::make_shared<GL::Texture>("textures/generic/black.png", GL::TextureFormat::PNG);
	GL::Material disk_material = GL::Material(disk_diffuse_texture, disk_specular_texture, disk_emissive_texture, 64.0f);
	auto disk_mesh = std::make_shared <GL::Mesh>(diskMeshGen->VBOBuffer, diskMeshGen->EBOBuffer);
	GL::Model disk_model = GL::Model(disk_mesh);
	GL::Renderable diskRenderable = GL::Renderable(std::move(disk_model), std::move(disk_material), simulationShader);
	diskRenderable.model.mesh->sendVBOBuffer();
	diskRenderable.model.mesh->sendEBOBuffer();

	glm::mat4 diskModelMatrix = glm::mat4(1.0f);
	diskRenderable.modelMatrix = diskModelMatrix;
	diskRenderable.projectionMatrix = SimpleCamera::computeProjectionMatrix(screenSettings);

	//listeObjectCollisionable.emplace_back(diskMeshGen.get());














	// -------------------------------------------------------------------------------------------

	

	SimulationState commandVariables{
	.tissu = LeTissu,
	};
	std::thread consoleThread(commandThread, &commandVariables);

	int recordingFrameNumber = 0;
	// -----------------------------------------------------------------------------------------------
	while (!glfwWindowShouldClose(window))
	{
		// ---------------------------------------------------------------------------------
		// Calcul FPS
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		// Compter les frames
		frameCount++;
		fpsTimeAccumulator += deltaTime;
		// Afficher FPS toutes les secondes
		if (fpsTimeAccumulator >= 1.0) {
			std::string title = "Simulation Tissu (FPS: " + std::to_string(frameCount) + ")";
			glfwSetWindowTitle(window, title.c_str());
			frameCount = 0;
			fpsTimeAccumulator = 0.0;
		}

		// ---------------------------------------------------------------------------------
		// Initialisation fenetre
		processInput(window);

		glClearColor(0.1f, 0.15f, 0.15f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ---------------------------------------------------------------------------------
		// Calcul physique (tout est dans physic.h, ou presque :p )
		if (!commandVariables.pauseSimulation) {
			transfereData(commandVariables, physicsSettings);
			glfwSetMouseButtonCallback(window, Grabbing::mouse_button_callback);
			Grabbing::mouse_calculation(window, LeTissu, tissuRenderable, screenSettings);

			Physics::physicCalculationSubsteped(LeTissu, listeObjectCollisionable, physicsSettings);

			if (frameCount % 6 == 0) {  //Calcul des normals toutes les 6 frames, gain de perf
				LeTissu.calculationNormals();
			}
			LeTissu.generateVBOBuffer();
		}
		glm::vec3 cameraPos;
		if (commandVariables.pauseCamera) {
			SimpleCamera::CameraSettings tempCameraSettings = cameraSettings;
			tempCameraSettings.rotationSpeed = 0.0f;
			cameraPos = SimpleCamera::computeCameraPos(tempCameraSettings);
		}
		else {
			cameraPos = SimpleCamera::computeCameraPos(cameraSettings);
		}
		
		simulationShader.sendUniformVec3("viewPos", cameraPos);

		Light::sendToShader(lightSettings, simulationShader);

		// ---------------------------------------------------------------------------------
		// Recording to OBJ
		std::string recordingDirectory = "recording";
		if (commandVariables.recordingEnabled) {
			exportFrameAsOBJ(LeTissu.positions, LeTissu.EBOBuffer, recordingDirectory, recordingFrameNumber);
			recordingFrameNumber++;
		}
		else {
			recordingFrameNumber = 0;
		}
		

		// ---------------------------------------------------------------------------------
		// Draw tissu
		tissuRenderable.shader.use();
		tissuRenderable.model.mesh->VBOBuffer = LeTissu.VBOBuffer;
		tissuRenderable.model.mesh->sendVBOBuffer();
		tissuRenderable.viewMatrix = SimpleCamera::computeViewMatrix(cameraPos, cameraSettings.target);
		tissuRenderable.wireframeDisplay = commandVariables.tissuWireframeEnabled;
		if (commandVariables.tissuEnabled) 
			tissuRenderable.draw();

		// ---------------------------------------------------------------------------------
		// Draw Sphere
		sphereRenderable.shader.use();
		sphereRenderable.viewMatrix = SimpleCamera::computeViewMatrix(cameraPos, cameraSettings.target);
		sphereRenderable.wireframeDisplay = commandVariables.sphereWireframeEnabled;
		if (commandVariables.sphereEnabled)
			sphereRenderable.draw();
		

		// ---------------------------------------------------------------------------------
		// Draw Box
		boxRenderable.shader.use();
		boxRenderable.viewMatrix = SimpleCamera::computeViewMatrix(cameraPos, cameraSettings.target);
		boxRenderable.wireframeDisplay = commandVariables.boxWireframeEnabled;
		if (commandVariables.boxEnabled)
			boxRenderable.draw();

		//// ---------------------------------------------------------------------------------
		//// Draw Cylinder
		//cylinderRenderable.shader.use();
		//cylinderRenderable.shader.sendUniformVec3("viewPos", cameraPos);
		//cylinderRenderable.updateViewMatrix(computeViewMatrix(cameraPos, cameraSettings.target));
		//cylinderRenderable.sendMatrix();
		//Light::sendToShader(lightSettings, cylinderRenderable.shader);
		//cylinderRenderable.draw();

		// ---------------------------------------------------------------------------------
		// Draw Cone
		coneRenderable.shader.use();
		coneRenderable.viewMatrix = SimpleCamera::computeViewMatrix(cameraPos, cameraSettings.target);
		coneRenderable.wireframeDisplay = commandVariables.coneWireframeEnabled;
		if (commandVariables.coneEnabled)
			coneRenderable.draw();

		// ---------------------------------------------------------------------------------
		// Draw Disk (Floor)
		diskRenderable.shader.use();
		diskRenderable.viewMatrix = SimpleCamera::computeViewMatrix(cameraPos, cameraSettings.target);
		diskRenderable.wireframeDisplay = commandVariables.diskWireframeEnabled;
		if (commandVariables.diskEnabled)
			diskRenderable.draw();

		// ---------------------------------------------------------------------------------
		//if (commandVariables.windEnabled)
		//	std::cout << commandVariables.windEnabled << std::endl; //applyWind(tissu);


		// ---------------------------------------------------------------------------------
		// Cleaning
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	running = false;
	consoleThread.join();
	glfwTerminate();
	return 0;
}




// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}