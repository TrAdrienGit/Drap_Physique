#ifndef VERTEX_H
#define VERTEX_H
#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include <memory>

// ----------------------------------------------------------------------------------------------------

class Vertex
{
public:
	// ------------------------------------------------------------------------------------------------
	// Vertex Attributes
	glm::vec3 acceleration = glm::vec3(0.0f);
	glm::vec3 vitesse = glm::vec3(0.0f);
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 positionOld = glm::vec3(0.0f); //Permet l'intégration de Verlet (de calculer deltaX, donc la vitesse)

	glm::vec3 normal = glm::vec3(0.0f);
	glm::vec2 UV = glm::vec2(0.0f);

	glm::vec3 color = glm::vec3(1.0f);
	
	std::vector<std::weak_ptr<Vertex>> linkedVertices; // pour éviter les cycles de dependance
	
	// Physics Attributes
	bool isPhysics = true;
	bool isAffectedGravity = true;
	bool isAffectedTension = true;
	bool isFixed = false;

	// ------------------------------------------------------------------------------------------------
	// Constructor
	Vertex() {}
	Vertex(glm::vec3 position) : position(position), positionOld(position) {}
	
	// ------------------------------------------------------------------------------------------------
	// Utility Methods
	glm::vec3 calculVitesse() {
		this->vitesse = position - positionOld;
		return this->vitesse;
	}
	void addLinkedVertex(const std::shared_ptr<Vertex>& neighbor) {
		// 1. Vérifie que ce n’est pas soi-même
		if (neighbor.get() == this) {
			std::cerr << "Tentative d’ajouter un sommet à lui-même comme voisin." << std::endl;
			return;
		}

		// 2. Vérifie si le voisin est déjà dans la liste
		for (const auto& weakVtx : linkedVertices) {
			if (auto vtx = weakVtx.lock()) {
				if (vtx == neighbor) {
					// Déjà présent
					return;
				}
			}
		}

		// 3. Ajoute le voisin
		linkedVertices.emplace_back(neighbor);
	}
	
	// Debug Methods
	void printPoint() {
		std::cout << "Point Position:" << std::endl;
		std::cout << glm::to_string(position) << std::endl;
	}
	void printVoisin() {
		std::cout << linkedVertices.size() << " total Neighbors" << std::endl;
		for (size_t o = 0; o < linkedVertices.size(); o++) {
			std::cout << "Neighbor [" << o << "] | ";
			if (auto voisin = linkedVertices[o].lock()) { //.lock converti le weak_ptr en shared_ptr dans ce scope
				voisin->printPoint();
			}
		}
	}
	void printNormal() {
		std::cout << "Point Normal:" << std::endl;
		std::cout << glm::to_string(normal) << std::endl;
	}
	void printAllInfo() {
		printPoint();
		printNormal();
		printVoisin();
	}
};
#endif