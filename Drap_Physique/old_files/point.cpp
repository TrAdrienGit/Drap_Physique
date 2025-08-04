#define GLM_ENABLE_EXPERIMENTAL
#include "point.h"
#include "triangle.h"
#include <glm/gtx/string_cast.hpp>

Point::Point(GLfloat x, GLfloat y, GLfloat z, glm::vec3 normal, glm::vec3 vitesse, glm::vec3 acceleration)
{
	vecPos = glm::vec3(x, y, z);
	vecPosAncienne = glm::vec3(x, y, z);
	vecVit = vitesse;
	vecAccel = acceleration;
	vecNormal = normal;
}

void Point::updateNormal()
{
	glm::vec3 tempVec = glm::vec3(0.0f, 0.0f, 0.0f);
	for (auto& t : linkedSurfaces)
	{
		tempVec += t->vecNormal;
	}
	vecNormal = glm::normalize(tempVec);
}

glm::vec3 Point::calculVitesse()
{
	return (vecPos - vecPosAncienne);
}

void Point::printPoint()
{
	std::cout << "Point Position:" << std::endl;
	std::cout << glm::to_string(vecPos) << std::endl;
}

void Point::printVoisin()
{
	std::cout << listVoisin.size() << " total Neighbors" << std::endl;
	for (size_t o = 0; o < listVoisin.size(); o++) {
		std::cout << "Neighbor [" << o << "] | ";
		listVoisin[o]->printPoint();
	}
}

void Point::printNormal()
{
	std::cout << "Point Normal:" << std::endl;
	std::cout << glm::to_string(vecNormal) << std::endl;
}

void Point::printAllInfo()
{
	printPoint();
	printNormal();
	printVoisin();
}
