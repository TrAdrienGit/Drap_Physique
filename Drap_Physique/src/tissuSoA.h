#ifndef TISSUSOA_H
#define TISSUSOA_H

#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include <memory>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/norm.hpp>
#include <omp.h>
#include <tuple>

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
		float gapPointStartX = 0.1f;
		float gapPointStartY = 0.1f;
		float floorHeight = -10.0f;
		float mass = 1.0f;
		TissuSettings()
		{
			gapPointStartX = sizeX / resolutionX;
			gapPointStartY = sizeY / resolutionY;
		}
	};

	// ----------------------------------------------------------------------------------------------------
	// Vertex Attributes
	class TissuSoA
	{
	public:
		// ------------------------------------------------------------------------------------------------
		// Attributes
		TissuSettings tissuSettings;

		std::vector<glm::vec3> positionsNew;
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> positionsOld;
		std::vector<glm::vec3> vitesses;
		std::vector<glm::vec3> accelerations;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> colors; //Not integrated
		std::vector<glm::vec3> masses; //Not integrated

		std::vector<std::vector<std::tuple<size_t, float>>> linkedVertices;   // size_t est l'indice du voision, le float sa distance-gap pour le ressort

		std::vector<bool> isPhysics; //Not used
		std::vector<bool> isAffectedGravity;
		std::vector<bool> isAffectedTension; //Not used
		std::vector<bool> isFixed;
		std::vector<bool> isGrabbed;

		int grabbedIndex = -1;
		float grabDepth = 10.0f;
		bool isCornerLocked = true;
		bool isSideLocked = true;

		// ---

		std::vector<float> VBOBuffer;
		std::vector<unsigned int> EBOBuffer;
		// ------------------------------------------------------------------------------------------------
		// Parsing function
		inline size_t getIndex(int x, int y) const {
			if (x < 0 || y < 0 ||
				x >= tissuSettings.resolutionX ||
				y >= tissuSettings.resolutionY) {
				throw std::out_of_range("getIndex: indices (x, y) OOB.");
			}
			return static_cast<size_t>(x * tissuSettings.resolutionY + y);
		}
		
		// ------------------------------------------------------------------------------------------------
		// Constructor
		TissuSoA(TissuSettings tissuSettings) : tissuSettings(tissuSettings)
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
			int count = tissuSettings.resolutionX * tissuSettings.resolutionY;

			positionsNew.resize(count);
			positions.resize(count);
			positionsOld.resize(count);
			vitesses.resize(count, glm::vec3(0.0f));
			accelerations.resize(count, glm::vec3(0.0f));
			normals.resize(count, glm::vec3(0.0f));
			uvs.resize(count);
			colors.resize(count);
			masses.resize(count);

			linkedVertices.resize(count);

			isPhysics.resize(count, true);
			isAffectedGravity.resize(count, true);
			isAffectedTension.resize(count, true);
			isFixed.resize(count, false);
			isGrabbed.resize(count, false);

			// ---

			for (size_t i = 0; i < this->tissuSettings.resolutionX; i++) {
				for (size_t j = 0; j < this->tissuSettings.resolutionY; j++) {
					size_t index = getIndex(i, j);

					positions[index] = glm::vec3(
						i * this->tissuSettings.gapPointStartX,
						j * this->tissuSettings.gapPointStartY,
						this->tissuSettings.startingHeight);
				}
			}
		}
		void calculationVertexNeighbors() // On stock les indices des points et pas des weak_ptr des points
		{
			for (size_t i = 0; i < this->tissuSettings.resolutionX; i++)
			{
				for (size_t j = 0; j < this->tissuSettings.resolutionY; j++)
				{
					size_t index = getIndex(i, j);
					if (i > 0)                                       // Gauche
						this->linkedVertices[index].emplace_back(std::tuple<size_t, float>{ getIndex(i - 1, j), tissuSettings.gapPointStartX });
					if (i < this->tissuSettings.resolutionX - 1)    // Droite
						this->linkedVertices[index].emplace_back(std::tuple<size_t, float>{ getIndex(i + 1, j), tissuSettings.gapPointStartX });
					if (j > 0)                                      // Bas
						this->linkedVertices[index].emplace_back(std::tuple<size_t, float>{ getIndex(i, j - 1), tissuSettings.gapPointStartY });
					if (j < this->tissuSettings.resolutionY - 1)    // Haut
						this->linkedVertices[index].emplace_back(std::tuple<size_t, float>{ getIndex(i, j + 1), tissuSettings.gapPointStartY });
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
			// Reset Normals
			for (size_t i = 0; i < this->tissuSettings.resolutionX; i++) {
				for (size_t j = 0; j < this->tissuSettings.resolutionY; j++) {
					size_t index = getIndex(i, j);
					this->normals[index] = glm::vec3(0.0f);
				}
			}

			// Accumulation Normals
			for (size_t i = 0; i < EBOBuffer.size(); i += 3)
			{
				unsigned int i0 = EBOBuffer[i];
				unsigned int i1 = EBOBuffer[i + 1];
				unsigned int i2 = EBOBuffer[i + 2];

				const glm::vec3& v0 = positions[i0];
				const glm::vec3& v1 = positions[i1];
				const glm::vec3& v2 = positions[i2];

				// Calcul de la normale du triangle
				glm::vec3 edge1 = v1 - v0;
				glm::vec3 edge2 = v2 - v0;
				glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

				// Addition aux normals des sommets (pour moyenne)
				normals[i0] += faceNormal;
				normals[i1] += faceNormal;
				normals[i2] += faceNormal;
			}

			// Normalisation finale
#			pragma omp parallel for
			for (int i = 0; i < (int)normals.size(); ++i)
				if (glm::length2(normals[i]) > glm::epsilon<float>())
					normals[i] = glm::normalize(normals[i]);

		}
		void calculationUVs()
		{
			for (size_t i = 0; i < this->tissuSettings.resolutionX; i++) {
				for (size_t j = 0; j < this->tissuSettings.resolutionY; j++) {
					size_t index = getIndex(i, j);
					this->uvs[index].x = float(i) / float(this->tissuSettings.resolutionX - 1);
					this->uvs[index].y = float(j) / float(this->tissuSettings.resolutionY - 1);
				}
			}
		}
		void generateVBOBuffer()
		{
			VBOBuffer.clear();
			VBOBuffer.reserve(this->tissuSettings.resolutionX * this->tissuSettings.resolutionY * 8);

			for (size_t i = 0; i < this->tissuSettings.resolutionX; i++) {
				for (size_t j = 0; j < this->tissuSettings.resolutionY; j++) {
					size_t index = getIndex(i, j);
					//VertexPosVec
					this->VBOBuffer.emplace_back(this->positions[index].x);
					this->VBOBuffer.emplace_back(this->positions[index].y);
					this->VBOBuffer.emplace_back(this->positions[index].z);
					//NormalVec
					this->VBOBuffer.emplace_back(this->normals[index].x);
					this->VBOBuffer.emplace_back(this->normals[index].y);
					this->VBOBuffer.emplace_back(this->normals[index].z);
					//TextureVec
					this->VBOBuffer.emplace_back(this->uvs[index].x);
					this->VBOBuffer.emplace_back(this->uvs[index].y);
				}
			}
		}

		// ------------------------------------------------------------------------------------------------
		// Utility Methods
		void updateVertexPosition()  // (0.0f) -> NewPos -> Pos -> OldPos -> *Void*
		{
			for (size_t i = 0; i < this->accelerations.size(); i++) {
				this->positionsOld[i] = this->positions[i];
				this->positions[i] = this->positionsNew[i];
				this->positionsNew[i] = glm::vec3(0.0f);
			}
		}
		void resetAcceleration() {
			for (size_t i = 0; i < this->accelerations.size(); i++) {
				this->accelerations[i] = glm::vec3(0.0f);
			}
		}
		void computeVitesses() {
			for (size_t i = 0; i < this->vitesses.size(); i++) {
				this->vitesses[i] = this->positions[i] - this->positionsOld[i];
			}
		}
		void lockCorner(bool boolean) {
			this->isFixed[this->getIndex(0, 0)] = boolean;
			this->isFixed[this->getIndex(0, this->tissuSettings.resolutionY - 1)] = boolean;
			this->isFixed[this->getIndex(this->tissuSettings.resolutionX - 1, 0)] = boolean;
			this->isFixed[this->getIndex(this->tissuSettings.resolutionX - 1, this->tissuSettings.resolutionY - 1)] = boolean;
			this->isCornerLocked = boolean;
			this->isSideLocked = boolean;
		}
		void lockSide(bool boolean) {
			this->isFixed[this->getIndex(0, this->tissuSettings.resolutionY - 1)] = boolean;
			this->isFixed[this->getIndex(this->tissuSettings.resolutionX - 1, this->tissuSettings.resolutionY - 1)] = boolean;
			this->isSideLocked = boolean;
		}
		void resetPosition() {
			for (size_t i = 0; i < this->tissuSettings.resolutionX; i++) {
				for (size_t j = 0; j < this->tissuSettings.resolutionY; j++) {
					size_t index = getIndex(i, j);

					positions[index] = glm::vec3(
						i * this->tissuSettings.gapPointStartX,
						j * this->tissuSettings.gapPointStartY,
						this->tissuSettings.startingHeight);
				}
			}
			for (size_t i = 0; i < this->vitesses.size(); i++) {
				this->positionsOld[i] = this->positions[i];
				this->positionsNew[i] = this->positions[i];
				this->vitesses[i] = glm::vec3(0.0f);
				this->accelerations[i] = glm::vec3(0.0f);
				this->isFixed[i] = false;
			}
			this->isCornerLocked = true;
			this->isSideLocked = true;
			lockCorner(isCornerLocked);
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
			for (int i = 0; i < this->tissuSettings.resolutionX; i++) {
				for (int j = 0; j < this->tissuSettings.resolutionY; j++) {
					size_t index = getIndex(i, j);
					std::cout << "[" << i << "," << j << "]: " << glm::to_string(positions[index]) << std::endl;
				}
			}
			std::cout << std::endl;
		}
		void printVertexNormal()
		{
			std::cout << "   Printing Vertex Normals:" << std::endl;
			for (int i = 0; i < this->tissuSettings.resolutionX; i++) {
				for (int j = 0; j < this->tissuSettings.resolutionY; j++) {
					size_t index = getIndex(i, j);
					std::cout << "[" << i << "," << j << "]: " << glm::to_string(normals[index]) << std::endl;
				}
			}
			std::cout << std::endl;
		}
		void printVertexUV()
		{
			std::cout << "   Printing Vertex UVs:" << std::endl;
			for (int i = 0; i < this->tissuSettings.resolutionX; i++) {
				for (int j = 0; j < this->tissuSettings.resolutionY; j++) {
					size_t index = getIndex(i, j);
					std::cout << "[" << i << "," << j << "]: " << glm::to_string(uvs[index]) << std::endl;
				}
			}
			std::cout << std::endl;
		}
	};
}


#endif