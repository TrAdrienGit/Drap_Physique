#ifndef SIMPLE_CAMERA_H
#define SIMPLE_CAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace SimpleCamera
{
	struct ScreenSettings {
		unsigned int width = 1280;
		unsigned int height = 720;
	};

	struct CameraSettings {
		glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
		float height = 3.0f;
		float rotationRadius = 1.5f * 10.0f;
		float rotationSpeed = 0.3f;
	};

	glm::vec3 computeCameraPos(CameraSettings& cameraSettings)
	{
		float camX = static_cast<float>(sin(glfwGetTime() * cameraSettings.rotationSpeed) * cameraSettings.rotationRadius);
		float camY = static_cast<float>(cos(glfwGetTime() * cameraSettings.rotationSpeed) * cameraSettings.rotationRadius);
		glm::vec3 cameraPos = glm::vec3(camX, camY, cameraSettings.height) + cameraSettings.target;
		return cameraPos;
	}

	glm::mat4 computeViewMatrix(glm::vec3 cameraPos, glm::vec3 cameraTarget)
	{
		glm::mat4 view = glm::mat4(1.0f);
		glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
		glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 right = glm::normalize(glm::cross(up, cameraDirection));
		glm::vec3 newUp = glm::cross(cameraDirection, right);
		view = glm::lookAt(cameraPos, cameraTarget, newUp);
		return view;
	}

	glm::mat4 computeProjectionMatrix(ScreenSettings screenSettings)
	{
		return glm::perspective(glm::radians(45.0f), (float)screenSettings.width / (float)screenSettings.height, 0.1f, 1000.0f);
	}





}








#endif