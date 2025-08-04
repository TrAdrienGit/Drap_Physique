#ifndef POINT_H
#define POINT_H
#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <iostream>


class Triangle; // Déclaration anticipée

class Point
{
public:
	glm::vec3 vecPos;
	glm::vec3 vecPosAncienne; //Permet l'intégration de Verlet (de calculer deltaX, donc la vitesse)
	glm::vec3 vecVit;
	glm::vec3 vecAccel;
	std::vector<std::shared_ptr<Point>> listVoisin;
	std::vector<std::shared_ptr<Triangle>> linkedSurfaces;
	glm::vec3 vecNormal;
	bool isPhysics = true;
	bool isAffectedGravity = true;
	bool isAffectedTension = true;
	bool isFixed = false;

	Point(GLfloat x, GLfloat y, GLfloat z, glm::vec3 normal = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 vitesse = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 acceleration = glm::vec3(0.0f, 0.0f, 0.0f));
	void updateNormal();
	glm::vec3 calculVitesse();
	void printPoint();
	void printVoisin();
	void printNormal();
	void printAllInfo();
};

#endif