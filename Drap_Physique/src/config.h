#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "tissuSoA.h"
#include "simple_camera.h"
#include "light.h"
#include "physicSoA.h"

// ----------------------------------------------------------------------------------------------------
// Quick Test
bool debug;

bool activateSphere;
bool activateBox;
bool activateCylinder;
bool activateCone;

bool lockCorner;

// -------------------------------------------------------------------------------------------
//                Simulation Settings
// Tissu Settings
Tissu::TissuSettings tissuSettings;
// Screen Settings
SimpleCamera::ScreenSettings screenSettings;
// Camera Settings
SimpleCamera::CameraSettings cameraSettings;
// Light Settings
Light::LightSettings lightSettings;
// Physics Settings
Physics::PhysicsSettings physicsSettings;

// -------------------------------------------------------------------------------------------

namespace Config
{
	void loadFromJSON(const std::string& path) {
		std::ifstream file(path);
		if (!file.is_open()) {
			std::cerr << "Erreur: impossible d'ouvrir le fichier " << path << std::endl;
			return;
		}

		nlohmann::json j;
		file >> j;

		// -------
		debug = j["debug"];
		activateSphere = j["activateSphere"];
		activateBox = j["activateBox"];
		activateCylinder = j["activateCylinder"];
		activateCone = j["activateCone"];
		lockCorner = j["lockCorner"];
		// -------
		tissuSettings.sizeX = j["tissu"]["sizeX"];
		tissuSettings.sizeY = j["tissu"]["sizeY"];
		tissuSettings.mass = j["tissu"]["mass"];
		tissuSettings.resolutionX = j["tissu"]["resolutionX"];
		tissuSettings.resolutionY = j["tissu"]["resolutionY"];
		tissuSettings.startingHeight = j["tissu"]["startingHeight"];
		tissuSettings.floorHeight = j["tissu"]["floorHeight"];
		tissuSettings.gapPointStartX = tissuSettings.sizeX / tissuSettings.resolutionX;
		tissuSettings.gapPointStartY = tissuSettings.sizeY / tissuSettings.resolutionY;
		// -------
		screenSettings.width = j["screen"]["width"];
		screenSettings.height = j["screen"]["height"];
		// -------
		cameraSettings.target = glm::vec3(j["tissu"]["sizeX"]/2, j["tissu"]["sizeY"]/2, j["camera"]["target"][2]);
		cameraSettings.height = j["camera"]["height"];
		cameraSettings.rotationSpeed = j["camera"]["rotationSpeed"];
		cameraSettings.rotationRadius = j["camera"]["rotationRadius"];
		// -------		
		lightSettings.ambient = glm::vec3(
			j["light"]["ambient"][0],
			j["light"]["ambient"][1],
			j["light"]["ambient"][2]
		);
		lightSettings.diffuse = glm::vec3(
			j["light"]["diffuse"][0],
			j["light"]["diffuse"][1],
			j["light"]["diffuse"][2]
		);
		lightSettings.specular = glm::vec3(
			j["light"]["specular"][0],
			j["light"]["specular"][1],
			j["light"]["specular"][2]
		);
		lightSettings.position = glm::vec3(
			j["light"]["position"][0],
			j["light"]["position"][1],
			j["light"]["position"][2]
		);
		// -------
		physicsSettings.gravitySettings.amplitude = j["physics"]["gravity"]["amplitude"];
		physicsSettings.gravitySettings.directionVector = glm::vec3(
			j["physics"]["gravity"]["directionVector"][0],
			j["physics"]["gravity"]["directionVector"][1],
			j["physics"]["gravity"]["directionVector"][2]
		);
		physicsSettings.gravitySettings.isEnabled = j["physics"]["gravity"]["isEnabled"];

		physicsSettings.tensionSettings.longeurVideRessortX = tissuSettings.gapPointStartX;
		physicsSettings.tensionSettings.longeurVideRessortY = tissuSettings.gapPointStartY;
		physicsSettings.tensionSettings.force = j["physics"]["tension"]["force"];
		physicsSettings.tensionSettings.maxTensionForce = j["physics"]["tension"]["maxTensionForce"];
		physicsSettings.tensionSettings.damping = j["physics"]["tension"]["damping"];
		physicsSettings.tensionSettings.isEnabled = j["physics"]["tension"]["isEnabled"];

		physicsSettings.collisionSettings.stiffness = j["physics"]["collision"]["stiffness"];
		physicsSettings.collisionSettings.antiClippingGap = j["physics"]["collision"]["antiClippingGap"];
		physicsSettings.collisionSettings.isEnabled = j["physics"]["collision"]["isEnabled"];

		physicsSettings.windSettings.amplitude = j["physics"]["wind"]["amplitude"];
		physicsSettings.windSettings.directionVector = glm::vec3(
			j["physics"]["wind"]["directionVector"][0],
			j["physics"]["wind"]["directionVector"][1],
			j["physics"]["wind"]["directionVector"][2]
		);
		physicsSettings.windSettings.oscillationFrequency = j["physics"]["wind"]["oscillationFrequency"];
		physicsSettings.windSettings.isOscillate = j["physics"]["wind"]["isOscillate"];
		physicsSettings.windSettings.isEnabled = j["physics"]["wind"]["isEnabled"];

		physicsSettings.temporalSettings.dt = j["physics"]["temporal"]["dt"];
		physicsSettings.temporalSettings.substeps = j["physics"]["temporal"]["substeps"];

		physicsSettings.grabSettings.force = j["physics"]["grab"]["force"];
		physicsSettings.grabSettings.damping = j["physics"]["grab"]["damping"];
		physicsSettings.grabSettings.isEnabled = j["physics"]["grab"]["isEnabled"];

		physicsSettings.selfCollisionSettings.minDistance = j["physics"]["selfCollision"]["minDistance"];
		physicsSettings.selfCollisionSettings.cellSize = j["physics"]["selfCollision"]["cellSize"];
		physicsSettings.selfCollisionSettings.isEnabled = j["physics"]["selfCollision"]["isEnabled"];
		// -------
	}




}
#endif