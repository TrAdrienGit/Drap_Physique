#define GLM_ENABLE_EXPERIMENTAL
#include "triangle.h"
#include "point.h" // On inclut ici la vraie définition

#include <iostream>
#include <glm/gtx/string_cast.hpp>

Triangle::Triangle(std::shared_ptr<Point> p0, std::shared_ptr<Point> p1, std::shared_ptr<Point> p2)
	: Sommet0(p0), Sommet1(p1), Sommet2(p2)
{
	vecNormal = glm::normalize(glm::cross(p1->vecPos - p0->vecPos, p2->vecPos - p0->vecPos));
}

void Triangle::updateNormal()
{
	vecNormal = glm::normalize(glm::cross(Sommet1->vecPos - Sommet0->vecPos, Sommet2->vecPos - Sommet0->vecPos));
}

void Triangle::printTriangle()
{
	std::cout << "    Triangle:" << std::endl;
	std::cout << "Vertex:" << std::endl;
	Sommet0->printPoint(); Sommet1->printPoint(); Sommet2->printPoint();
	std::cout << "Normal Vector:" << std::endl;
	std::cout << glm::to_string(vecNormal) << std::endl;
}