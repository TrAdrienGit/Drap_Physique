#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <glad/glad.h>
#include <memory>
#include <glm/glm.hpp>


class Point; // Déclaration anticipée

class Triangle
{
public:
	std::shared_ptr<Point> Sommet0;
	std::shared_ptr<Point> Sommet1;
	std::shared_ptr<Point> Sommet2;
	glm::vec3 vecNormal;

	Triangle(std::shared_ptr<Point> p0, std::shared_ptr<Point> p1, std::shared_ptr<Point> p2);
	void updateNormal();
	void printTriangle();
};


#endif