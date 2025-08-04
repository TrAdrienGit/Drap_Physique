#ifndef TISSU_H
#define TISSU_H

#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include <memory>
#include "vertex.h"
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/norm.hpp>

// ----------------------------------------------------------------------------------------------------

namespace Tissu
{
	// ------------------------------------------------------------------------------------------------
	// Settings Struct
	struct TissuSettings
	{
		glm::vec3 positionOrigin = glm::vec3(0.0f);
		float sizeX = 10.8f;
		float sizeY = 10.8f;
		unsigned int resolutionX = 5;
		unsigned int resolutionY = 5;
		GLfloat startingHeight = 0.5f;
		float gapPointStart = 0.1f;
		float floorHeight = -10.0f;
		float mass = 1.0f;
		TissuSettings()
		{
			gapPointStart = sizeX / resolutionX;
		}
	};

	// ----------------------------------------------------------------------------------------------------
	// Vertex Attributes
	class Tissu
	{
	public:
		// ------------------------------------------------------------------------------------------------
		// Attributes
		TissuSettings tissuSettings;

		std::vector<std::vector<std::shared_ptr<Vertex>>> listVertices;

		std::vector<float> VBOBuffer;
		std::vector<unsigned int> EBOBuffer;

		// ------------------------------------------------------------------------------------------------
		// Constructor
		Tissu(TissuSettings tissuSettings) : tissuSettings(tissuSettings)
		{
			Initialisation();
			calculationVertexNeighbors();
			generateEBOBuffer();
			calculationNormals();
			calculationUVs();
			generateVBOBuffer();
		}

		// ------------------------------------------------------------------------------------------------
		// Generation Methods
		void Initialisation()
		{
			this->listVertices.reserve(this->tissuSettings.resolutionX);

			for (size_t i = 0; i < this->tissuSettings.resolutionX; i++) {
				std::vector<std::shared_ptr<Vertex>> TempVector = {};
				for (size_t j = 0; j < this->tissuSettings.resolutionY; j++) {
					TempVector.emplace_back(std::make_shared<Vertex>(glm::vec3(
						i * this->tissuSettings.gapPointStart,
						j * this->tissuSettings.gapPointStart,
						this->tissuSettings.startingHeight)));
				}
				this->listVertices.emplace_back(TempVector);
			}
		}
		void calculationVertexNeighbors()
		{
			for (size_t i = 0; i < this->tissuSettings.resolutionX; i++) 
			{
				for (size_t j = 0; j < this->tissuSettings.resolutionY; j++) 
				{
					if (i > 0)                                       // Gauche
						this->listVertices[i][j]->linkedVertices.emplace_back(this->listVertices[i - 1][j]);
					if (i < this->tissuSettings.resolutionX - 1)    // Droite
						this->listVertices[i][j]->linkedVertices.emplace_back(this->listVertices[i + 1][j]);
					if (j > 0)                                      // Bas
						this->listVertices[i][j]->linkedVertices.emplace_back(this->listVertices[i][j - 1]);
					if (j < this->tissuSettings.resolutionY - 1)    // Haut
						this->listVertices[i][j]->linkedVertices.emplace_back(this->listVertices[i][j + 1]);
				}
			}
		}
		void generateEBOBuffer()
		{
			EBOBuffer.clear();
			EBOBuffer.reserve((tissuSettings.resolutionX - 1) * (tissuSettings.resolutionY - 1) * 6);

			for (size_t i = 0; i < tissuSettings.resolutionX - 1; ++i) {
				for (size_t j = 0; j < tissuSettings.resolutionY - 1; ++j) {
					size_t i0 = i + j * tissuSettings.resolutionX;
					size_t i1 = (i + 1) + j * tissuSettings.resolutionX;
					size_t i2 = (i + 1) + (j + 1) * tissuSettings.resolutionX;
					size_t i3 = i + (j + 1) * tissuSettings.resolutionX;

					// Triangle 1
					EBOBuffer.emplace_back(i0);
					EBOBuffer.emplace_back(i2);  // inversé pour normale correcte
					EBOBuffer.emplace_back(i1);

					// Triangle 2
					EBOBuffer.emplace_back(i0);
					EBOBuffer.emplace_back(i3);
					EBOBuffer.emplace_back(i2);
				}
			}
		}
		void calculationNormals()
		{
			std::vector<std::shared_ptr<Vertex>> tempListVertexFlattened;
			tempListVertexFlattened.reserve(tissuSettings.resolutionX * tissuSettings.resolutionY);

			for (size_t i = 0; i < this->tissuSettings.resolutionX; i++) {
				for (size_t j = 0; j < this->tissuSettings.resolutionY; j++) {
					this->listVertices[i][j]->normal = glm::vec3(0.0f);  // Reset normals
					tempListVertexFlattened.emplace_back(this->listVertices[i][j]);
				}
			}

			for (size_t i = 0; i < EBOBuffer.size(); i += 3)
			{
				unsigned int i0 = EBOBuffer[i];
				unsigned int i1 = EBOBuffer[i + 1];
				unsigned int i2 = EBOBuffer[i + 2];

				const glm::vec3& v0 = tempListVertexFlattened[i0]->position;
				const glm::vec3& v1 = tempListVertexFlattened[i1]->position;
				const glm::vec3& v2 = tempListVertexFlattened[i2]->position;

				// Calcul de la normale du triangle
				glm::vec3 edge1 = v1 - v0;
				glm::vec3 edge2 = v2 - v0;
				glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

				// Addition aux normales des sommets (pour moyenne)
				tempListVertexFlattened[i0]->normal += faceNormal;
				tempListVertexFlattened[i1]->normal += faceNormal;
				tempListVertexFlattened[i2]->normal += faceNormal;
			}

			// Normalisation finale
			for (auto& n : tempListVertexFlattened)
			{
				if (glm::length2(n->normal) > glm::epsilon<float>()) // évite division par zéro
					n->normal = glm::normalize(n->normal);
			}

		}
		void calculationUVs()
		{
			for (size_t i = 0; i < this->tissuSettings.resolutionX; i++) {
				for (size_t j = 0; j < this->tissuSettings.resolutionY; j++) {
					this->listVertices[i][j]->UV.x = float(i) / float(this->tissuSettings.resolutionX - 1);
					this->listVertices[i][j]->UV.y = float(j) / float(this->tissuSettings.resolutionY - 1);
				}
			}
		}
		void generateVBOBuffer()
		{
			VBOBuffer.clear();
			VBOBuffer.reserve(this->tissuSettings.resolutionX * this->tissuSettings.resolutionY * 8);

			for (size_t i = 0; i < this->tissuSettings.resolutionX; i++) {
				for (size_t j = 0; j < this->tissuSettings.resolutionY; j++) {
					//VertexPosVec
					this->VBOBuffer.emplace_back(this->listVertices[i][j]->position.x);
					this->VBOBuffer.emplace_back(this->listVertices[i][j]->position.y);
					this->VBOBuffer.emplace_back(this->listVertices[i][j]->position.z);
					//NormalVec
					this->VBOBuffer.emplace_back(this->listVertices[i][j]->normal.x);
					this->VBOBuffer.emplace_back(this->listVertices[i][j]->normal.y);
					this->VBOBuffer.emplace_back(this->listVertices[i][j]->normal.z);
					//TextureVec
					this->VBOBuffer.emplace_back(this->listVertices[i][j]->UV.x);
					this->VBOBuffer.emplace_back(this->listVertices[i][j]->UV.y);
				}
			}
		}

		// ------------------------------------------------------------------------------------------------
		// Utility Methods
		void downloadNewVertexPosition(const Tissu& temp_tissu)
		{
			for (size_t i = 0; i < this->tissuSettings.resolutionX; i++) {
				for (size_t j = 0; j < this->tissuSettings.resolutionY; j++) {
					this->listVertices[i][j]->position = temp_tissu.listVertices[i][j]->position;
				}
			}
		}
		void downloadAncienneVertexPosition(const Tissu& temp_tissu)
		{
			for (size_t i = 0; i < this->tissuSettings.resolutionX; i++) {
				for (size_t j = 0; j < this->tissuSettings.resolutionY; j++) {
					this->listVertices[i][j]->positionOld = temp_tissu.listVertices[i][j]->positionOld;
				}
			}
		}
		void updateAncienneVertexPosition()
		{
			for (size_t i = 0; i < this->tissuSettings.resolutionX; i++) {
				for (size_t j = 0; j < this->tissuSettings.resolutionY; j++) {
					this->listVertices[i][j]->positionOld = this->listVertices[i][j]->position;
				}
			}
		}

		// ------------------------------------------------------------------------------------------------
		// Debug Methods
		void printVBOBuffer() {
			std::cout << "\nPrinting VBO Buffer" << std::endl;
			for (size_t h = 0; h < this->VBOBuffer.size(); h += 8) {
				std::cout << "Vertex Pos: (" << this->VBOBuffer[h] << "/" << this->VBOBuffer[h + 1] << "/" << this->VBOBuffer[h + 2] << ")" << std::endl;
				std::cout << "  Normal:(" << this->VBOBuffer[h + 3] << "/" << this->VBOBuffer[h + 4] << "/" << this->VBOBuffer[h + 5] << ")" << std::endl;
				std::cout << "  Texture:(" << this->VBOBuffer[h + 6] << "/" << this->VBOBuffer[h + 7] << ")" << std::endl;
			}
			std::cout << "\n" << std::endl;
		}
		void printEBOBuffer() {
			std::cout << "\nPrinting EBO Buffer" << std::endl;
			for (size_t h = 0; h < this->EBOBuffer.size(); h += 3) {
				std::cout << "(" << this->EBOBuffer[h] << "-" << this->EBOBuffer[h + 1] << "-" << this->EBOBuffer[h + 2] << ")" << std::endl;
			}
			std::cout << "\n" << std::endl;
		}
		void printVertexPosition()
		{
			std::cout << "   Printing Vertex Positions:" << std::endl;
			for (int i = 0; i < listVertices.size(); i++)
			{
				for (int j = 0; j < listVertices[i].size(); j++)
				{
					std::cout << glm::to_string(listVertices[i][j]->position) << std::endl;
				}
			}
			std::cout << std::endl;
		}
		void printVertexNormal()
		{
			std::cout << "   Printing Vertex Normals:" << std::endl;
			for (int i = 0; i < listVertices.size(); i++)
			{
				for (int j = 0; j < listVertices[i].size(); j++)
				{
					std::cout << glm::to_string(listVertices[i][j]->normal) << std::endl;
				}
			}
			std::cout << std::endl;
		}
		void printVertexUV()
		{
			std::cout << "   Printing Vertex UVs:" << std::endl;
			for (int i = 0; i < listVertices.size(); i++)
			{
				for (int j = 0; j < listVertices[i].size(); j++)
				{
					std::cout << glm::to_string(listVertices[i][j]->UV) << std::endl;
				}
			}
			std::cout << std::endl;
		}
	};
}


#endif