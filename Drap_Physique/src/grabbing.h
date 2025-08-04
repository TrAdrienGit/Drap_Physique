#ifndef GRABBING_H
#define GRABBING_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "tissuSoA.h"
#include "glencapsulation.h"
#include "simple_camera.h"

// ----------------------------------------------------------------------------------------------------

namespace Grabbing
{
	// ------------------------------------------------------------------------------------------------
	// Globals
	double mouseX, mouseY;
	glm::vec3 rayOrigin; //Camera Pos
	glm::vec3 rayDirection;
	bool isGrabbing = false;

	// ------------------------------------------------------------------------------------------------
	// Settings Struct
	//struct GrabContext {
	//	Tissu::TissuSoA& tissu;
	//	GL::Renderable& tissuRenderable;
	//};

	// ------------------------------------------------------------------------------------------------
	// Functions
	
	void getRayFromMouse(
		double mouseX, double mouseY,
		int screenWidth, int screenHeight,
		const glm::mat4& viewMatrix,
		const glm::mat4& projectionMatrix,
		glm::vec3& rayOrigin,
		glm::vec3& rayDirection)
	{
		// Calcule un rayon 3D depuis la position écran de la souris

		// Convertir la position de la souris en Normalized Device Coordinates (NDC) x[-1;1] y[-1;1] z = 1 pointe vers le fond du frustum
		float x = (2.0f * (float)mouseX) / screenWidth - 1.0f;
		float y = 1.0f - (2.0f * (float)mouseY) / screenHeight; // OpenGL inverse l'axe Y
		float z = 1.0f;

		glm::vec3 ray_nds = glm::vec3(x, y, z);

		// Clip space (ajoute la coordonnée w = 1.0)
		//On forme un vecteur 4D pour passer à l’étape inverse de la projection
		//Le - 1.0f pour z indique que le rayon sort de l'écran (vers la scène)
		//Le w vaut 1.0 : standard pour un point
		glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0f, 1.0f);

		// Transforme en vue caméra (eye space)
		//On annule l'effet de la projection perspective
		//w = 0.0 : il s agit d'un vecteur directinnel maintenant
		glm::vec4 ray_eye = glm::inverse(projectionMatrix) * ray_clip;
		ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

		// Transforme en espace monde
		glm::vec4 ray_world4 = glm::inverse(viewMatrix) * ray_eye;
		glm::vec3 ray_world = glm::normalize(glm::vec3(ray_world4));

		// Origine du rayon = position caméra
		//Cette position se trouve dans la 4e colonne de la matrice view inversée
		rayOrigin = glm::vec3(glm::inverse(viewMatrix)[3]);
		rayDirection = ray_world;
	}


	int findClosestVertexToRay(
		const std::vector<glm::vec3>& positions,
		const std::vector<bool>& isFixed,
		const glm::vec3& rayOrigin,
		const glm::vec3& rayDirection,
		float maxDistance = 0.05f)
	{
		int closestIndex = -1;
		float closestDistance = maxDistance;

		// En gros: on a un vecteur rayon qui part de la camera(rayOrigin) vers la rayDirection.
		// Un autre vecteur entre le point de tissu et la camera
		// on fait une projection du 2nd vec sur le 1er
		// et on calcul la distance entre le point et le point au bout du vecteur projeté
		for (int i = 0; i < (int)positions.size(); ++i) {
			if (isFixed[i]) continue; // Ignore les points déjà fixés

			const glm::vec3& point = positions[i];
			glm::vec3 toPoint = point - rayOrigin;
			float t = glm::dot(toPoint, rayDirection); // projection sur le rayon

			if (t < 0.0f) continue; // ignore les points derrière la caméra

			glm::vec3 closestPointOnRay = rayOrigin + t * rayDirection;
			float distance = glm::length(point - closestPointOnRay);

			if (distance < closestDistance) {
				closestDistance = distance;
				closestIndex = i;
			}
		}

		return closestIndex; // -1 si rien trouvé
	}


	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			isGrabbing = true;
		}

		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			isGrabbing = false;
		}
	}

	void mouse_calculation(GLFWwindow* window, Tissu::TissuSoA& tissu, GL::Renderable& tissuRenderable, SimpleCamera::ScreenSettings screenSettings)
	{
		if (isGrabbing)
		{
			// Calcul du rayon souris
			glfwGetCursorPos(window, &mouseX, &mouseY);
			getRayFromMouse(mouseX, mouseY, screenSettings.width, screenSettings.height, tissuRenderable.viewMatrix, tissuRenderable.projectionMatrix, rayOrigin, rayDirection);
			int idx = findClosestVertexToRay(tissu.positions, tissu.isFixed, rayOrigin, rayDirection, 0.5f);
			
			if (idx != -1 && tissu.grabbedIndex == -1) {
				tissu.grabbedIndex = idx;
				glm::vec3 camPos = glm::vec3(glm::inverse(tissuRenderable.viewMatrix)[3]);
				tissu.grabDepth = glm::length(tissu.positions[idx] - camPos); // distance réelle
				//tissu.isFixed[idx] = true;
			}
		}
		else
		{
			if (tissu.grabbedIndex != -1) {
				tissu.isFixed[tissu.grabbedIndex] = false;
			}
			tissu.grabbedIndex = -1;
		}
	}
}
























#endif