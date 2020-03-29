// TeslaGamesEngine.cpp : This file contains the 'main' function. Program execution begins and ends there.
// The TeslaGamesEngine will be changed to a .lib, not .exe and then be included in the TeslaGames Project

#include "pch.h"
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <memory>
#include <list>

// Rendering includes
#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtx\rotate_vector.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
#include "Model.h"
#include "Skybox.h"

//Health stuff
#include "Combat.h"
#include "HealthComponent.h"
#include "DamagingObject.h"
#include "Vehicle.h"
#include "Game.h"

//AudioStuff
#include "AudioEngine.h"
#include "AudioBoomBox.h"

//Controller stuff
#include "Controller.h"

//PhysX and Physics Engine
#include "PhysicsEngine.h"

//UI stuff
#include "HUDcreator.h"
#include "StartScreen.h"
#include "Menu.h"
#include "ReadyScreen.h"
#include "PauseScreen.h"

//Shadow stuff
#include "Shadow.h"

//Ability stuff
#include "Caltrops.h"
#include "Bullet.h"

// AI stuff
#include "AIDrivingComponent.h"
#include "AIShootingComponent.h"

// Stuff for imgui
#include "imGui/imgui.h"
#include "imGui/imgui_impl_glfw.h"
#include "imGui/imgui_impl_opengl3.h"

// end of stuff for imgui

struct localAxis {
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;
};


/* Rendering variables */
const float toRadians = 3.14159265f / 180.0f;

Skybox skybox;

GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
uniformSpecularIntensity = 0, uniformShininess = 0;

//sWindow mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
std::vector<HUD*> HUDList;
std::vector<Shadow*> shadowList;
//std::list<std::unique_ptr<Caltrops>> caltropsList;											//using list instead of vector since we will often insert and delete elements in the list
std::list<std::unique_ptr<Bullet>> bulletsList;
std::list <ShootComp*> shootList;
//std::list<std::unique_ptr<ShootComp*>> shootList;

Camera camera;

Shader shadowShader;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;

//sahow textures
Texture shadowTexture;

Material shinyMaterial;
Material dullMaterial;

PhysicsEngine* physEng;

//Player 1 objs
//Model TeslaCar;
Model Teslacar_chasis;
Model T_turret;
//Player 2 objs
Model Teslacar2_chasis;
Model T2_turret;
//Player 3 objs
Model Teslacar3_chasis;
Model T3_turret;
//Player 4 objs
Model Teslacar4_chasis;
Model T4_turret;

//AI Objs
Model TeslaCarAI;
Model TeslacarAI_chasis;
Model TAI_turret;

Model racetrack;
Model racetrack_walls;
Model racetrack_floor;
Model bulletobj;
Model boxTest;
Model oil;

//Pickup models
Model defense_pickup;
Model ammo_pickup;

Model drivingPointModel;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];


//helper variables
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

bool isCameraFlipped = false;

float shoot_distance_x = 0; // Bullet vector movement for x
float shoot_distance_y = 0; // Bullet vector movement for y
float shoot_distance_z = 0; //Bullet vector movement for z
float bullet_speed = 0.5f;  //velocity of bullet when traversing

float bullet_boundary = 15;

//Mesh positioning and rotation debugging for player/car obj (current position for CAR)
float pos_x = 0;
float pos_y = 0;
float pos_z = 0;


glm::vec3 car_front;

// Vertex Shader
static const char* vShader = "Shaders/shader.vert";

// Fragment Shader
static const char* fShader = "Shaders/shader.frag";

//Vertex shader of shadow_shader
static const char* vShadow_shader = "Shaders/shadow_shader.vert";

//Fragment shader of shadow_shader
static const char* fShadow_shader = "Shaders/shadow_shader.frag";


struct yawPitch {
	float yaw;
	float pitch;
};

void update(localAxis a, float yaw, float pitch) {
	a.front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	a.front.y = sin(glm::radians(pitch));
	a.front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	a.front = glm::normalize(a.front);

	a.right = glm::normalize(glm::cross(a.front, glm::vec3(0,1,0)));
	a.up = glm::normalize(glm::cross(a.right, a.front));
}

/* End of rendering variables */

void CreateShaders()
{
	Shader *shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
	
	shadowShader.createShadowFromFiles(vShadow_shader, fShadow_shader);
}

void createShadows() {
	unsigned int shadowIndecis[] = {
		0, 1, 3,
		2, 1, 3
	};

	GLfloat shadowVertices[] = {
		-1.95f, -0.8f, -2.0f,		0.0, 0.0,
		-1.95f, -0.8f, 2.0f,		0.0, 1.0,
		2.05f, -0.8f, 2.0f,		1.0, 1.0,
		2.05f, -0.8f, -2.0f,		1.0, 0.0
	};

	Shadow* shadow = new Shadow();
	shadow->createShadow(shadowVertices, shadowIndecis, 20, 6);
	shadowList.push_back(shadow);
}

// A function to obtain input, called each frame
//add vehicle movement to these FOR NOW
//TO DO: Someone comment all the controls for each button
void parseControllerInput(Controller* controller)
{
	// Update controller object with current input MUST BE FIRST
	controller->update();

	if (controller->isButtonPressed(XButtons.B)) {
		if (!controller->LStick_InDeadzone()) {
			physEng->player->handbrakeTurn(0.9f, controller->leftStick_X());
		}
		else {
			physEng->player->handbrakeTurn(0.9f, 0.f);
		}
		//std::cout << controller->getIndex() << " " << "B PRESSED and HELD" << std::endl;
	}
	else {
		physEng->player->handbrakeTurn(0.0f, 0.f);
	}

	isCameraFlipped = (controller->isButtonPressed(XButtons.Y));
	
	if (controller->isButtonDown(XButtons.Start)) {
		gameFlag = false;
		pauseFlag = true;
	}

	//press up to suicide (for testing health bar)
	if (controller->isButtonDown(XButtons.DPad_Up)) {
		physEng->player->update_health();

		std::cout << "Current health: " << physEng->player->getHealthComponent()->GetHealth() << std::endl;
	}
	
	//Sticks and triggers
	if (!controller->LStick_InDeadzone()) {
		//physEng.turn(controller->leftStick_X());
		float value = controller->leftStick_X();
		physEng->player->turn(value);
	}
	else {
		physEng->player->turn(0.f);
	}

	if (controller->rightTrigger() > 0.0 || controller->leftTrigger() > 0.0) {
		if (controller->rightTrigger() > 0.0) {
			physEng->player->forwards(controller->rightTrigger());
		}
		if (controller->leftTrigger() > 0.0) {
			physEng->player->reverse(controller->leftTrigger());
		}
	}
	else if (controller->rightTrigger() == 0.0 && controller->leftTrigger() == 0.0) {
		physEng->player->reverse(0.0f);
		physEng->player->forwards(0.0f);
	}
	

	// Update the gamepad for next frame MUST BE LAST
	controller->refreshState();
}

void RenderScene()
{
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.5f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	brickTexture.UseTexture();
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 4.0f, -2.5f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dirtTexture.UseTexture();
	dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[1]->RenderMesh();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dirtTexture.UseTexture();
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[2]->RenderMesh();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-7.0f, 0.0f, 10.0f));
	model = glm::scale(model, glm::vec3(0.006f, 0.006f, 0.006f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
}

void RenderPass(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
	glViewport(0, 0, 1366, 768);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	skybox.DrawSkybox(viewMatrix, projectionMatrix);
	shaderList[0].UseShader();

	uniformModel = shaderList[0].GetModelLocation();
	uniformProjection = shaderList[0].GetProjectionLocation();
	uniformView = shaderList[0].GetViewLocation();
	uniformModel = shaderList[0].GetModelLocation();
	uniformEyePosition = shaderList[0].GetEyePositionLocation();
	uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
	uniformShininess = shaderList[0].GetShininessLocation();

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

	shaderList[0].SetDirectionalLight(&mainLight);

	shaderList[0].Validate();

	shaderList[0].UseShader();

	uniformModel = shaderList[0].GetModelLocation();
	uniformProjection = shaderList[0].GetProjectionLocation();
	uniformView = shaderList[0].GetViewLocation();
	uniformModel = shaderList[0].GetModelLocation();
	uniformEyePosition = shaderList[0].GetEyePositionLocation();
	uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
	uniformShininess = shaderList[0].GetShininessLocation();

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

	shaderList[0].SetDirectionalLight(&mainLight);

	glm::vec3 lowerLight = camera.getCameraPosition();
	lowerLight.y -= 0.3f;
	spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

	shaderList[0].Validate();

	RenderScene();
}

int main()
{
	const char* glsl_version = "#version 130"; // USED FOR IMGUI SETTING
	mainWindow = Window(1280, 720);
	mainWindow.Initialise();

	//bool startScreenFlag = true;
	//int menuFlag = 0;
	bool setupGame = true;

	HUDcreator hud;
	hud.load();

	StartScreen startScreen;
	startScreen.load();
	Menu menu;
	menu.load();
	ReadyScreen readyScreen;
	readyScreen.load();
	PauseScreen pauseScreen;
	pauseScreen.load();


	physEng = new PhysicsEngine();

	//physEng->createPickupTriggerVolume(18, -2, -67, 4, 4, 4);

	Renderer r = Renderer(mainWindow, camera);

	// Rendering setup
	CreateShaders();
	createShadows();

	camera = Camera(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 20.0f, -20.0f, 5.0f, 2.f);
	yawPitch yp;
	yp.yaw = 90.f;
	yp.pitch = -20.0f;


	bool winFlag = false;
	bool loseFlag = false;

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureAlpha();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureAlpha();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureAlpha();
	shadowTexture = Texture("Textures/shadow.png");
	shadowTexture.LoadTextureAlpha();

	
	shinyMaterial = Material(4.0f, 256);
	dullMaterial = Material(0.3f, 4);

	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.5f, 0.5f,
		0.0f, 0.0f, -1.0f);

	unsigned int pointLightCount = 0;
	pointLights[0] = PointLight(0.0f, 0.0f, 1.0f,
								0.0f, 0.1f,
								0.0f, 0.0f, 0.0f,
								0.3f, 0.2f, 0.1f);
	//pointLightCount++;
	pointLights[1] = PointLight(0.0f, 1.0f, 0.0f,
								0.0f, 0.1f,
								-4.0f, 2.0f, 0.0f,
								0.3f, 0.1f, 0.1f);
	//pointLightCount++;

	unsigned int spotLightCount = 0;
	spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
						0.0f, 2.0f,
						0.0f, 0.0f, 0.0f,
						0.0f, -1.0f, 0.0f,
						1.0f, 0.0f, 0.0f,
						20.0f);
	spotLightCount++;
	spotLights[1] = SpotLight(1.0f, 1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, -1.5f, 0.0f,
		-100.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		20.0f);
	spotLightCount++;

	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);


	std::vector<std::string> skyboxFaces;
	///*
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");
	//*/
	/*
	skyboxFaces.push_back("Textures/Skybox/Sunny_01A_right.jpg");
	skyboxFaces.push_back("Textures/Skybox/Sunny_01A_left.jpg");
	skyboxFaces.push_back("Textures/Skybox/Sunny_01A_up.jpg");
	skyboxFaces.push_back("Textures/Skybox/Sunny_01A_down.jpg");
	skyboxFaces.push_back("Textures/Skybox/Sunny_01A_back.jpg");
	skyboxFaces.push_back("Textures/Skybox/Sunny_01A_front.jpg");
	*/
	skybox = Skybox(skyboxFaces);

	//Loading TeslaTrucks models
	////////////////////////////////////////////////////////////////////////
	Teslacar_chasis.LoadModel("Models/TeslaGamesTruck2_yellowchasis.obj");
	T_turret.LoadModel("Models/TeslaGamesTruck2_yellowturret.obj");

	Teslacar2_chasis.LoadModel("Models/TeslaGamesTruck2_bluechasis.obj");
	T2_turret.LoadModel("Models/TeslaGamesTruck2_blueturret.obj");

	Teslacar3_chasis.LoadModel("Models/TeslaGamesTruck2_greenchasis.obj");
	T3_turret.LoadModel("Models/TeslaGamesTruck2_greenturret.obj");

	Teslacar4_chasis.LoadModel("Models/TeslaGamesTruck2_purplechasis.obj");
	T4_turret.LoadModel("Models/TeslaGamesTruck2_purpleturret.obj");

	TeslacarAI_chasis.LoadModel("Models/TeslaGamesTruck2_redchasis.obj");
	TAI_turret.LoadModel("Models/TeslaGamesTruck2_redturret.obj");
	//////////////////////////////////////////////////////////////////////////
	boxTest.LoadModel("Models/wall.obj");
	//caltrop.LoadModel("Models/caltrops.obj");
	//racetrack.LoadModel("Models/track2.obj");
	
	
	bulletobj.LoadModel("Models/bullet.obj");
	defense_pickup.LoadModel("Models/defense_box.obj");
	ammo_pickup.LoadModel("Models/ammo_box.obj");

	oil.LoadModel("Models/oil.obj");

	drivingPointModel.LoadModel("Models/bullet.obj");
	// TODO: Put FPS code into Game.Play()
	// Loop until window closed

	glfwSwapInterval(1);
	
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings

	ImGui_ImplGlfw_InitForOpenGL(mainWindow.getWindow(), true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Imgui setting END

	//Audio system setup
	AudioEngine audioSystem = AudioEngine();

	AudioBoomBox mainMenuMusic = audioSystem.createBoomBox(audioConstants::SOUND_FILE_TTG_MAIN_MENU);
	AudioBoomBox raceMusic; 

	mainMenuMusic.setVolume(0.4f);
	mainMenuMusic.loopSound(true);

	//Controller
	Controller player1 = Controller(1);
	Controller player2 = Controller(2);

	bool P1Connected = player1.isConnected();
	bool P2Connected = player2.isConnected();

	std::cout << "Player1 connected: " << P1Connected << std::endl;
	std::cout << "Player2 connected: " << P2Connected << std::endl;


	//This does all the Ai stuff as far TeslaGameEngine is concerned
	Track raceTrack;
	std::vector<AIShootingComponent> aiShootingComponents;

	glm::vec3 front = glm::normalize(glm::vec3(0.f, -0.5f, 1.f));
	camera.setFront(front.x, front.y, front.z);

	while (!mainWindow.getShouldClose()) {
		if (closeWindowFlag) {
			mainWindow.setWindowClose();
			continue;
		}

		while (startScreenFlag) {
			if (!mainMenuMusic.isSoundPlaying()) {
				mainMenuMusic.playSound();
			}
			if (fromGameFlag) {
				if (glfwGetTime() - backTime > 0.5) {
					startScreen.loadController(&player1);
					fromGameFlag = false;
				}
			}
			else
				startScreen.loadController(&player1);

			startScreen.use();

			mainWindow.swapBuffers();
		}

		while (menuFlag) {
			menu.loadController(&player1);
			menu.use();
			mainWindow.swapBuffers();
		}

		while (readyScreenFlag) {
			readyScreen.loadController(&player1);
			readyScreen.use();
			mainWindow.swapBuffers();
		}

		while (pauseFlag) {
			audioSystem.pauseAllActiveSources();
			pauseScreen.loadController(&player1);
			pauseScreen.use();
			mainWindow.swapBuffers();
		}

		if (setupGame == true) {
			//Reset this variable to reset the game
			physEng->initAudioForVehicles(&audioSystem);
			camera.initializeAudio(&audioSystem);

			setupGame = false;

			int gameMode = menu.getSelectedGameMode();
			int trackNum = menu.getSelectedTrack();
			int AINum = menu.getSelectedNumOfAI();
			std::cout << trackNum << std::endl;

			if (trackNum == trackTypeConstants::HYPERLOOP) {
				racetrack_walls.LoadModel("Models/track2finalwalls.obj", physEng->gPhysics, physEng->gCooking, physEng->gMaterial, physEng->gScene, false);
				racetrack_floor.LoadModel("Models/track2finalfloor.obj", physEng->gPhysics, physEng->gCooking, physEng->gMaterial, physEng->gScene, true);

				raceMusic = audioSystem.createBoomBox(audioConstants::SOUND_FILE_TTG_RACE_HYPERLOOP);
				raceMusic.setVolume(0.35f);
				raceMusic.loopSound(true);
			}
			else if (trackNum == trackTypeConstants::STARLINK) {
				racetrack_walls.LoadModel("Models/track2final_Twalls.obj", physEng->gPhysics, physEng->gCooking, physEng->gMaterial, physEng->gScene, false);
				racetrack_floor.LoadModel("Models/track2final_Tfloor.obj", physEng->gPhysics, physEng->gCooking, physEng->gMaterial, physEng->gScene, true);

				raceMusic = audioSystem.createBoomBox(audioConstants::SOUND_FILE_TTG_RACE_STARLINK);
				raceMusic.setVolume(0.35f);
				raceMusic.loopSound(true);
			}

			raceTrack.initializeTrackPoints(trackNum);
			physEng->initAITrack(&raceTrack);

			for (int i = 0; i < AINum; i++) {
				physEng->addEnemyVehicle(i);
			}
			physEng->addPlayerVehicle(AINum + player1.getIndex());

			if (trackNum == trackTypeConstants::STARLINK) {
				for (auto AI : physEng->enemyVehicles) {
					AI->incrementAITrackPoint();
				}
			}

			std::vector<Vehicle*> vehicles;
			vehicles.push_back(physEng->player);
			std::vector<Vehicle*> aiVehicles = physEng->enemyVehicles;
			vehicles.insert(vehicles.end(), aiVehicles.begin(), aiVehicles.end());
			shaderList[0].UseShader();
			for (auto ai : aiVehicles) {
				AIShootingComponent aiShooting = AIShootingComponent(ai);
				aiShooting.SetVehicles(vehicles);
				aiShooting.SetUniformLocations(shaderList[0].GetModelLocation(), shaderList[0].GetSpecularIntensityLocation(), shaderList[0].GetShininessLocation());
				aiShootingComponents.push_back(aiShooting);
			}
		}

		while (gameFlag)
		{
			if (audioSystem.allHaveBeenPaused == true) {
				audioSystem.resumeAllActiveSources();
			}
			if (mainMenuMusic.isSoundPlaying()) {
				mainMenuMusic.stopSound();
			}
			if (!raceMusic.isSoundPlaying()) {
				raceMusic.playSound();
			}

			physEng->stepPhysics();

			for (int i = 0; i < aiShootingComponents.size(); i++) {
				aiShootingComponents[i].Aim();
			}

			const physx::PxVehicleDrive4W* vehicle = physEng->player->gVehicle4W;	//get vehicle
			const physx::PxRigidDynamic* vDynamic = vehicle->getRigidDynamicActor();
			physx::PxQuat vehicleQuaternion = vDynamic->getGlobalPose().q;
			physx::PxVec3 v_dir = vehicleQuaternion.getBasisVector2();
			const physx::PxVec3 vehiclePositionPhysx = vDynamic->getGlobalPose().p;
			glm::vec3 vehiclePosition(vehiclePositionPhysx.x, vehiclePositionPhysx.y, vehiclePositionPhysx.z);

			GLfloat now = glfwGetTime();
			deltaTime = now - lastTime;
			lastTime = now;


			// Get + Handle User Input
			glfwPollEvents();

			//camera.keyControl(mainWindow.getsKeys(), deltaTime);
			//camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

			// Clear the window
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			//CAMERA RENDERING
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//update camera
			//now distance and other stuff are inside camera class
			glm::vec3 dir = glm::normalize(glm::vec3(v_dir.x, 0, v_dir.z));
			glm::vec3 dirToUse;
			if (isCameraFlipped) {
				dirToUse = dir * -1.f;
			}
			else {
				dirToUse = dir;
			}

			if (player1.RStick_InDeadzone()) {
				camera.stickControl(0.f, vehiclePosition, dirToUse, player1.isButtonDown(XButtons.R_Thumbstick), isCameraFlipped);
			}
			else {
				camera.stickControl(player1.rightStick_X(), vehiclePosition, dirToUse, player1.isButtonDown(XButtons.R_Thumbstick), isCameraFlipped);
			}


			//end camera stuff


			if (P1Connected)
				parseControllerInput(&player1);
			if (P2Connected)
				parseControllerInput(&player2);

			skybox.DrawSkybox(camera.calculateViewMatrix(), projection);

			// Setup shader
			shaderList[0].UseShader();
			uniformModel = shaderList[0].GetModelLocation();
			uniformProjection = shaderList[0].GetProjectionLocation();
			uniformView = shaderList[0].GetViewLocation();
			uniformEyePosition = shaderList[0].GetEyePositionLocation();
			uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
			uniformShininess = shaderList[0].GetShininessLocation();

			shaderList[0].SetDirectionalLight(&mainLight);
			shaderList[0].SetPointLights(pointLights, pointLightCount);
			shaderList[0].SetSpotLights(spotLights, spotLightCount);

			physx::PxVec3 carPos = physEng->player->GetPosition();	//position of TeslaCar

			glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
			glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

			shaderList[0].UseShader();
			uniformModel = shaderList[0].GetModelLocation();
			uniformProjection = shaderList[0].GetProjectionLocation();

			// Draw pyramid one
			glm::mat4 model = glm::mat4(1.0f);

			//RACING TRACK
			//////////////////////////////////////////////////////////////////////////


			// Draw racing track
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, -5.f, -3.2));
			//model = glm::scale(model, glm::vec3(20.f, 20.f, 20.f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
			//racetrack.RenderModel();
			racetrack_walls.RenderModel();
			racetrack_floor.RenderModel();

			for (int i = 0; i < raceTrack.listOfPoints.size(); i++) {
				TrackDrivingPoint point = *raceTrack.listOfPoints.at(i);
				model = model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(point.x, point.y, point.z));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				drivingPointModel.RenderModel();
			}

			//please use linked list instead of vector if you will delete stuff from the list at random position very often, linked list will save much more time

			//test with 2 pickup boxes
			auto pickup = physEng->pickupBoxes.begin();
			while (pickup != physEng->pickupBoxes.end()) {
				//if it is picked up, delete it from the list
				//it looks like it won't be triggered once it is deleted, but I think it's still in the triggerActor list
				if ((*pickup)->getIsPicked()) {
					physEng->pickupBoxes.erase(pickup++);
				}
				else {
					physx::PxVec3 wallPos = (*pickup)->actor->getGlobalPose().p;
					glm::vec3 wallp(wallPos.x, wallPos.y + 1.f, wallPos.z);
					model = glm::mat4(1.0f);
					model = glm::translate(model, wallp);
					//Consider making the pickup boxes a hardcoded size and hardcoding the 
					//trigger volumes to be the same size
					model = glm::scale(model, glm::vec3(1.1f, 0.3f, 0.2f));	//keep these scale values!
					glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
					shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
					//boxTest.RenderModel();
					defense_pickup.RenderModel();
					++pickup;
				}
			}

			physx::PxVec3 forwardvec = physx::PxVec3(vehicleQuaternion.x, 0, vehicleQuaternion.z);	//holds camera vectors that match the car

			physx::PxVec3  Direction = vehicleQuaternion.getBasisVector2();

			/////////////////////////////////////////////////////////////////////////////////
					//BULLLETS AND PLAYING SHOOTING SOUND

			ShootComp* ba = physEng->player->getShootingComponent();
			HealthComponent* ha = physEng->player->getHealthComponent();
		
			glm::vec3 camDir = camera.getCameraDirection();

			//Temporary condition for player that shows when player DIES, need to replace with something that eliminates all components
			if ((ha->GetHealth()) > 0) {
				//Draw bullets after Refactor. If affected by smoke they cant shoot
				if ((player1.isButtonDown(XButtons.R_Shoulder) || player1.isButtonDown(XButtons.L_Shoulder)) && !physEng->player->affectedBySmoke) {
					if (isCameraFlipped) {
						ba->fire(vehiclePosition, uniformModel, uniformSpecularIntensity, uniformShininess, Direction.x, Direction.y, Direction.z);
					}
					else {
						ba->fire(vehiclePosition, uniformModel, uniformSpecularIntensity, uniformShininess, camDir.x, Direction.y, camDir.z);
					}
				 }
				ba->renderAllBullets();
				//payer1->renderBullets();

					physx::PxMat44 modelMat(vDynamic->getGlobalPose());	//make model matrix from transform of rigid dynamic
					modelMat.scale(physx::PxVec4(0.3f, 0.3f, 0.3f, 1.f));	//scales the model
					glUniformMatrix4fv(uniformModel, 1, GL_FALSE, modelMat.front());

				shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				//TeslaCar.RenderModel();
				Teslacar_chasis.RenderModel();
		
				// Reset model
				model = glm::mat4(1.0f);
				glm::vec3 y_rot(0.0,1.0,0.0); //axis of rotation
				glm::vec3 tem = glm::vec3(modelMat.getPosition().x, modelMat.getPosition().y, modelMat.getPosition().z); 
				model = glm::translate(model, tem); //Update turret pos with position of vehicle
				model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f)); //resize turret
				// TODO: This moves to ShootComp class
				float angleAroundY = glm::degrees(atan2(camDir.x, camDir.z)); //calculate angle of rotation
				float angletoUse = angleAroundY * 3.14 / 180; //convert to radians
				model = glm::rotate(model, angletoUse, y_rot);

				if (isCameraFlipped) {
					glUniformMatrix4fv(uniformModel, 1, GL_FALSE, modelMat.front());
				}
				else {
					glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				}
			
				T_turret.RenderModel(); //renders turret	
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		

			
			//CALTROPS
			//when dpad down is pushed, make a new caltrop and trigger volume
			if (player1.isButtonDown(XButtons.DPad_Down) && !physEng->player->affectedBySmoke) {
				PxVec3 p(physEng->player->GetPosition());
				physEng->createCaltropsTriggerVolume(p.x, p.y, p.z, 2.5f, 2, 2.5f);
			}

			auto c = physEng->caltropsList.begin();
			while (c != physEng->caltropsList.end()) {	//remove dead caltrops
				if ((*c)->isDead()) {
					physEng->gScene->removeActor(*((*c)->actor));
					physEng->caltropsList.erase(c++);
				}
				else {
					(*c)->load(uniformModel, uniformSpecularIntensity, uniformShininess);
					(*c)->renderCaltrops();
					++c;
				}
			}

			//caltrops end here

			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			//OIL
			//when dpad down is pushed, make a new caltrop and trigger volume
			if (player1.isButtonDown(XButtons.DPad_Right) && !physEng->player->affectedBySmoke) {
				PxVec3 p(physEng->player->GetPosition());
				physEng->createOilTriggerVolume(p.x, p.y, p.z, 2.5f, 2, 2.5f);
			}

			auto o = physEng->oilList.begin();
			while (o!= physEng->oilList.end()) {	//remove dead caltrops
				if ((*o)->isDead()) {
					physEng->gScene->removeActor(*((*o)->actor));
					physEng->oilList.erase(o++);
				}
				else {
					(*o)->load(uniformModel, uniformSpecularIntensity, uniformShininess);
					(*o)->renderOil();
					++o;
				}
			}

			//oil ends here

			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			//SMOKE
			//when dpad left is pushed, make a new smoke and trigger volume
			if (player1.isButtonDown(XButtons.DPad_Left) && !physEng->player->affectedBySmoke) {
				PxVec3 p(physEng->player->GetPosition());
				physEng->createSmokeTriggerVolume(p.x, p.y, p.z, 2.5f, 2, 2.5f);
			}

			auto s = physEng->smokeList.begin();
			while (s != physEng->smokeList.end()) {	//remove dead smoke
				if ((*s)->isDead()) {
					physEng->gScene->removeActor(*((*s)->actor));
					physEng->smokeList.erase(s++);
				}
				else {
					(*s)->load(uniformModel, uniformSpecularIntensity, uniformShininess);
					(*s)->renderSmoke();
					++s;
				}
			}

			//smoke ends here

			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			//Enemy CARS rendering
			//there is probably a much better way of rendering the other enemy cars, but this works for now
			if (!physEng->enemyVehicles.empty()) {
				for (int i = 0; i < physEng->enemyVehicles.size(); i++) {


					Vehicle* v = physEng->enemyVehicles.at(i);
					HealthComponent* th = v->getHealthComponent();
					//Temporal condition that "Destroys" the object; should create a method that destroys The vehicles and handle stuff
					if (th->GetHealth() > 10) {

					const physx::PxVehicleDrive4W* enemyV = v->gVehicle4W;	//get vehicle
					const physx::PxRigidDynamic* enemyvDynamic = enemyV->getRigidDynamicActor();
					physx::PxQuat enemyvehicleQuaternion = enemyvDynamic->getGlobalPose().q;
					physx::PxVec3 enemyv_dir = enemyvehicleQuaternion.getBasisVector2();
					const physx::PxVec3 enemyvehiclePositionPhysx = enemyvDynamic->getGlobalPose().p;
					glm::vec3 enemyvehiclePosition(enemyvehiclePositionPhysx.x, enemyvehiclePositionPhysx.y, enemyvehiclePositionPhysx.z);

					physx::PxMat44 enemymodelMat(enemyvDynamic->getGlobalPose());	//make model matrix from transform of rigid dynamic
					enemymodelMat.scale(physx::PxVec4(0.3f, 0.3f, 0.3f, 1.f));	//scales the model
					glUniformMatrix4fv(uniformModel, 1, GL_FALSE, enemymodelMat.front());

					shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);

						//std::cout<<"Current vehicle health is:  " << th->GetHealth() << std::endl;
						TeslacarAI_chasis.RenderModel();

						model = glm::mat4(1.0f);
						glm::vec3 y_rot(0.0, 1.0, 0.0); //axis of rotation
						glm::vec3 tem = glm::vec3(enemymodelMat.getPosition().x, enemymodelMat.getPosition().y, enemymodelMat.getPosition().z);
						model = glm::translate(model, tem); //Update turret pos with position of vehicle
						model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f)); //resize turret
						auto aimdir_x = v->getShootingComponent()->Direction_x;
						auto aimdir_z = v->getShootingComponent()->Direction_z;
						float angleAroundY = glm::degrees(atan2(aimdir_x, aimdir_z)); //calculate angle of rotation
						float angletoUse = angleAroundY * 3.14 / 180; //convert to radians
						model = glm::rotate(model, angletoUse, y_rot);
						glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
						TAI_turret.RenderModel();
					}
				}
			}

			//Render enemy bullets
			for (auto ai : physEng->enemyVehicles) {
				ai->getShootingComponent()->renderAllBullets();
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


			//SHADOW RENDERING
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			model = glm::mat4(1.0f);
			glm::vec3 tem = glm::vec3(53, -2, -83);
			model = glm::translate(model, tem); //Update turret pos with position of vehicle
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

			//turn on blend mode
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			shadowShader.UseShader();
			uniformModel = shadowShader.GetModelLocation();
			uniformProjection = shadowShader.GetProjectionLocation();
			uniformView = shadowShader.GetViewLocation();

			//player's vehicle's shadow
			model = glm::mat4(1.0f);
			physx::PxMat44 carModel(vDynamic->getGlobalPose());	//make model matrix from transform of rigid dynamic
			//modelMat.scale(physx::PxVec4(0.3f, 0.3f, 0.3f, 1.f));	//scales the model
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, carModel.front());


			glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
			//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

			shadowTexture.UseTexture();
			shadowList[0]->renderShadow();

			//ai vehicles' shadows
			for (auto v : physEng->enemyVehicles) {
				physx::PxMat44 AIcarModel(v->GetTransform());

				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, AIcarModel.front());
				glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
				//glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

				shadowTexture.UseTexture();
				shadowList[0]->renderShadow();
			}

			glEnable(GL_DEPTH_TEST);

			//Shadows end here
			//HUD
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//Rendering HUD
			//HUD stars here

			//get lose or win
			if (physEng->player->numLaps == 5)
				winFlag = true;
			else
				for (auto v : physEng->enemyVehicles)
					if (v->numLaps == 5)
						loseFlag = true;

			if (winFlag == true)
				hud.setGameState(true);
			if (loseFlag == true)
				hud.setGameState(false);
			if (winFlag)
				hud.setLapNumber(physEng->player->numLaps);
			else
				hud.setLapNumber(physEng->player->numLaps + 1);


			hud.setAbilityNumber(physEng->player->ability);
			hud.setAliveNumber(physEng->enemyVehicles.size());
			//don't now how to get position right now
			//hud.setPositionNumber();
			hud.setBulletNum(physEng->player->getShootingComponent()->ammo);
			hud.setHealth(physEng->player->getHealthComponent()->GetHealth());

			hud.use();

			//HUD ends here

			glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

			// End of rendering 

			// Start the Dear ImGui frame

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			{
				static float f = 0.0f;
				static int counter = 0;

			ImGui::Begin("Debug");
			ImGui::Text("Driving mode and Position");
			ImGui::Text("Frame per Second counter");               // Display some text (you can use a format strings too)
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text(": %i Xpos: %.2f Ypos: %.2f Zpos: %.2f", physEng->getModeType(), carPos.x, carPos.y, carPos.z);
			//ImGui::Text("Drivemode: %i Xpos: %f Ypos: %f Zpos: %f", physEng->getModeType(), carPos.x, carPos.y, carPos.z);
			ImGui::Text("Camera Xvec: %f Yvec: %f Zvec: %f", camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
			ImGui::Text("Drivemode: %i Xvec: %f Yvec: %f Zvec: %f", physEng->getModeType(), v_dir.x, v_dir.y, v_dir.z);

				ImGui::End();
			}

			// Rendering imgui
			ImGui::Render();
			int display_w, display_h;
			glfwGetFramebufferSize(mainWindow.getWindow(), &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			//glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
			//glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
			//glClear(GL_COLOR_BUFFER_BIT);

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			// imgui ends here

			// TODO: Load shader in a material struct in the model (Basically all of the following code refactored to being in model

			mainWindow.swapBuffers();

			//Shooting actions
			if (mainWindow.getShouldClose() == true) {
				gameFlag = false;
			}
		}
	}
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	
	return 0;
}