#ifndef COLLISION_BOX_H
#define COLLISION_BOX_H

#include <iostream>
#include <glm/glm.hpp>
#include <memory>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/norm.hpp>
#include <cmath>

// ----------------------------------------------------------------------------------------------------

namespace MeshGenerator
{
	// ------------------------------------------------------------------------------------------------
	// Settings structs
	enum class PrimitiveType {
		BOX,
		SPHERE,
		CYLINDER,
		CONE,
		DISK,
	};
	struct MeshSettings {
		glm::vec3 position = glm::vec3(0.0f);
		float radius = 1.0f;
		int resolutionX = 8;
		int resolutionY = 8;
		int resolutionZ = 8;
		glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 size = glm::vec3(1.0f);
		float height = 1.0f;
		//bool alternateUV = false;
		//bool smoothShading = true;
	};
	struct GenerationSettings {
		bool generateNormals = true;
		bool generateUV = true;
		bool generateVBOBuffer = true;
	};

	// ------------------------------------------------------------------------------------------------
	// Main Generator
	class MeshGenerator
	{
	public:
		//Attributes
		PrimitiveType type = PrimitiveType::SPHERE;
		glm::vec3 meshCenter = glm::vec3(0.0f);
		std::vector<glm::vec3> vertexPositions;
		std::vector<glm::vec3> vertexNormals;
		std::vector<glm::vec2> vertexUV;
		std::vector<float> VBOBuffer;
		std::vector<unsigned int> EBOBuffer;

	protected:
		// Constructor
		MeshGenerator() = default;
		MeshGenerator(const glm::vec3& center, PrimitiveType type) : meshCenter(center), type(type) {}
	public:
		// Destructor
		virtual ~MeshGenerator() = default; // Nécessaire pour que dynamic_cast fonctionne

	public:
		// Generation Methods
		void generateVertexNormals()
		{
			vertexNormals.resize(vertexPositions.size(), glm::vec3(0.0f));

			for (size_t i = 0; i < EBOBuffer.size(); i += 3)
			{
				unsigned int i0 = EBOBuffer[i];
				unsigned int i1 = EBOBuffer[i + 1];
				unsigned int i2 = EBOBuffer[i + 2];

				const glm::vec3& v0 = vertexPositions[i0];
				const glm::vec3& v1 = vertexPositions[i1];
				const glm::vec3& v2 = vertexPositions[i2];

				// Calcul de la normale du triangle
				glm::vec3 edge1 = v1 - v0;
				glm::vec3 edge2 = v2 - v0;
				glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

				// Addition aux normales des sommets (pour moyenne)
				vertexNormals[i0] += faceNormal;
				vertexNormals[i1] += faceNormal;
				vertexNormals[i2] += faceNormal;
			}

			// Normalisation finale
			for (auto& n : vertexNormals)
			{
				if (glm::length2(n) > glm::epsilon<float>()) // évite division par zéro
					n = glm::normalize(n);
			}

		}
		void generateVBOBuffer()
		{
			if ((vertexPositions.size() != vertexNormals.size()) && (vertexUV.size() != vertexNormals.size()))
			{
				std::cerr << "Error: generateVBOBuffer: vertexPositions.size() != vertexNormals.size() != vertexUV.size()" << std::endl;
				std::cerr << "   vertexPositions.size()=" << vertexPositions.size() << std::endl;
				std::cerr << "   vertexNormals.size()=" << vertexNormals.size() << std::endl;
				std::cerr << "   vertexUV.size()=" << vertexUV.size() << std::endl;
			}
			else
			{
				this->VBOBuffer.clear();
				this->VBOBuffer.reserve(8 * vertexPositions.size());
				for (int i = 0; i < vertexPositions.size(); i++)
				{
					this->VBOBuffer.emplace_back(vertexPositions[i].x);
					this->VBOBuffer.emplace_back(vertexPositions[i].y);
					this->VBOBuffer.emplace_back(vertexPositions[i].z);

					this->VBOBuffer.emplace_back(vertexNormals[i].x);
					this->VBOBuffer.emplace_back(vertexNormals[i].y);
					this->VBOBuffer.emplace_back(vertexNormals[i].z);

					this->VBOBuffer.emplace_back(vertexUV[i].x);
					this->VBOBuffer.emplace_back(vertexUV[i].y);
				}
			}
		}

		// Utility Methods
		std::vector<float> flattenVec3Array(const std::vector<glm::vec3>& vec)
		{
			std::vector<float> temp_array = {};
			temp_array.reserve(vec.size() * 3); // Optimisation mémoire

			for (const auto& v : vec)
			{
				temp_array.emplace_back(v.x);
				temp_array.emplace_back(v.y);
				temp_array.emplace_back(v.z);
			}

			return temp_array;
		}
		float cleanFloat(float f, float eps = glm::epsilon<float>()) {
			return (std::abs(f) < eps) ? 0.0f : f;
		}
		glm::vec3 cleanVec3(glm::vec3 v) {
			return glm::vec3(cleanFloat(v.x), cleanFloat(v.y), cleanFloat(v.z));
		}

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
			for (glm::vec3 g : vertexPositions)
			{
				std::cout << glm::to_string(g) << std::endl;
			}
			std::cout << std::endl;
		}
		void printVertexNormal()
		{
			std::cout << "   Printing Vertex Normals:" << std::endl;
			for (glm::vec3 g : vertexNormals)
			{
				std::cout << glm::to_string(g) << std::endl;
			}
			std::cout << std::endl;
		}
		void printVertexUV()
		{
			std::cout << "   Printing Vertex UVs:" << std::endl;
			for (glm::vec2 g : vertexUV)
			{
				std::cout << glm::to_string(g) << std::endl;
			}
			std::cout << std::endl;
		}
	};

	// ------------------------------------------------------------------------------------------------
	// Sphere Generator
	class ProceduralSphereMesh : public MeshGenerator
	{
	public:
		float radius;
		int resolutionX;
		int resolutionY;
		glm::vec3 rotationAxis;
		std::vector<std::vector<glm::vec3>> tableSphereVertex = {};

		ProceduralSphereMesh(const MeshSettings& meshSettings, const GenerationSettings& genSettings)
			: MeshGenerator(meshSettings.position, PrimitiveType::SPHERE),
			radius(meshSettings.radius),
			resolutionX(meshSettings.resolutionX),
			resolutionY(meshSettings.resolutionY),
			rotationAxis(meshSettings.rotationAxis)
		{
			this->generate(genSettings);
		}
		void generate(const GenerationSettings& genSettings)
		{
			this->generateVertexPosition();
			this->generateIndice();
			if (genSettings.generateNormals) this->generateVertexNormals();
			if (genSettings.generateUV) this->generateSphericalUVs();
			if (genSettings.generateVBOBuffer && genSettings.generateNormals && genSettings.generateUV) this->generateVBOBuffer();
		}

	private:
		void generateVertexPosition()
		{
			float sphere_rayon = this->radius;
			glm::vec3 sphere_center = this->meshCenter;
			int nbPointTour = this->resolutionX;
			int nbCerclage = this->resolutionY;
			glm::mat3 rotationMatrix = glm::mat3_cast(glm::rotation(glm::vec3(0.0f, 0.0f, 1.0f), glm::normalize(this->rotationAxis)));

			vertexPositions.clear();
			vertexPositions.emplace_back(cleanVec3(rotationMatrix * glm::vec3(0.0f, 0.0f, -1.0f) * sphere_rayon + sphere_center)); //1st Point

			//check si nbPointTour >= 4
			//check si nbCerclge >= 3  (de pref nbCerclage impair)
			if (nbPointTour < 4) { nbPointTour = 4; }
			if (nbCerclage < 3) { nbCerclage = 3; }

			for (int i = 0 + 1; i < nbCerclage + 1; i++)
			{
				float circle_height = (i) * (sphere_rayon * 2) / (nbCerclage + 1);
				float circle_rayon = std::sqrt(std::pow(sphere_rayon, 2) - std::pow(circle_height - sphere_rayon, 2));
				float circle_iterator_step_angle = 2 * glm::pi<float>() / nbPointTour;
				float angleOffset = 0;
				if (i % 2 != 0) { angleOffset = circle_iterator_step_angle / 2; }
				for (int j = 0; j < nbPointTour; j++)
				{
					glm::vec3 point = glm::vec3(
						std::cos(circle_iterator_step_angle * j + angleOffset) * circle_rayon,
						std::sin(circle_iterator_step_angle * j + angleOffset) * circle_rayon,
						circle_height - sphere_rayon);

					vertexPositions.emplace_back(cleanVec3(rotationMatrix * point + sphere_center));
				}
			}
			vertexPositions.emplace_back(cleanVec3(rotationMatrix * glm::vec3(0.0f, 0.0f, 1.0f) * sphere_rayon + sphere_center)); //last Point
		}
		void generateIndice()
		{
			int nbPointTour = this->resolutionX;
			int nbCerclage = this->resolutionY;
			this->EBOBuffer.clear();

			//South Pole Triangles
			for (int i = 0; i < nbPointTour; i++)
			{
				this->EBOBuffer.emplace_back(0);
				this->EBOBuffer.emplace_back(i + 1);
				if (i + 2 >= nbPointTour + 1)
				{
					this->EBOBuffer.emplace_back(1);
				}
				else
				{
					this->EBOBuffer.emplace_back(i + 2);
				}

			}

			//Triangles between 2 circles
			for (int c = 0; c < nbCerclage - 1; c++)
			{
				if (c % 2 == 0)
				{
					for (int i = 0; i < nbPointTour - 1; i++)
					{
						this->EBOBuffer.emplace_back(i + nbPointTour * c + 1);
						this->EBOBuffer.emplace_back(i + nbPointTour * (c + 1) + 2);
						this->EBOBuffer.emplace_back(i + nbPointTour * c + 2);

						this->EBOBuffer.emplace_back(i + nbPointTour * c + 1);
						this->EBOBuffer.emplace_back(i + nbPointTour * (c + 1) + 1);
						this->EBOBuffer.emplace_back(i + nbPointTour * (c + 1) + 2);
					}

					this->EBOBuffer.emplace_back(nbPointTour * c + nbPointTour);
					this->EBOBuffer.emplace_back(nbPointTour * (c + 1) + 1);
					this->EBOBuffer.emplace_back(nbPointTour * c + 1);

					this->EBOBuffer.emplace_back(nbPointTour * c + nbPointTour);
					this->EBOBuffer.emplace_back(nbPointTour * (c + 1) + nbPointTour);
					this->EBOBuffer.emplace_back(nbPointTour * (c + 1) + 1);
				}
				else
				{
					for (int i = 0; i < nbPointTour - 1; i++)
					{
						this->EBOBuffer.emplace_back(i + nbPointTour * c + 1);
						this->EBOBuffer.emplace_back(i + nbPointTour * (c + 1) + 1);
						this->EBOBuffer.emplace_back(i + nbPointTour * c + 2);

						this->EBOBuffer.emplace_back(i + nbPointTour * c + 2);
						this->EBOBuffer.emplace_back(i + nbPointTour * (c + 1) + 1);
						this->EBOBuffer.emplace_back(i + nbPointTour * (c + 1) + 2);
					}

					this->EBOBuffer.emplace_back(nbPointTour * c + nbPointTour);
					this->EBOBuffer.emplace_back(nbPointTour * (c + 1) + nbPointTour);
					this->EBOBuffer.emplace_back(nbPointTour * c + 1);

					this->EBOBuffer.emplace_back(nbPointTour * c + 1);
					this->EBOBuffer.emplace_back(nbPointTour * (c + 1) + 1);
					this->EBOBuffer.emplace_back(nbPointTour * (c + 1) + nbPointTour);
				}

			}

			//North Pole Triangles
			for (int i = 0; i < nbPointTour; i++)
			{
				this->EBOBuffer.emplace_back(nbPointTour * (nbCerclage - 1) + i + 1);
				this->EBOBuffer.emplace_back(nbPointTour * nbCerclage + 1);
				if (i + 2 >= nbPointTour + 1)
				{
					this->EBOBuffer.emplace_back(nbPointTour * (nbCerclage - 1) + 1);
				}
				else
				{
					this->EBOBuffer.emplace_back(nbPointTour * (nbCerclage - 1) + i + 2);
				}

			}
		}
		void generateSphericalUVs()
		{
			// Mapping Sphérique(Spherical Mapping)
			// Il existe aussi Mapping Cube / Mapping Cylindrique / Environment Mapping / UV Atlasing
			vertexUV.reserve(vertexPositions.size());
			glm::mat3 rotationMatrix = glm::mat3_cast(glm::rotation(glm::vec3(0.0f, 0.0f, 1.0f), glm::normalize(this->rotationAxis)));
			glm::mat3 rotationMatrixInverse = glm::transpose(rotationMatrix);

			float prevU = 0.0f;
			bool first = true;

			for (const glm::vec3& pos : vertexPositions)
			{
				// Normaliser la position pour la mapper à la sphère unité
				glm::vec3 p = glm::normalize(rotationMatrixInverse * (pos - this->meshCenter));

				// Calcul de l'angle polaire (longitude)
				float u = 0.5f + atan2(p.z, p.x) / (2.0f * glm::pi<float>());

				// Calcul de l'angle azimutal (latitude)
				float v = 0.5f - asin(p.y) / glm::pi<float>();

				// gérer la couture sur U
				// Ajustement de la couture : si saut brutal entre deux UV consécutifs, on corrige U [0->1]
				if (!first && std::abs(u - prevU) > 0.5f)
				{
					if (u < 0.5f)
						u += 1.0f; // pousser u > 1 pour éviter le saut, on corrige ensuite dans le shader ou post-traitement
					else
						u -= 1.0f;
				}

				// atténuer la distorsion aux pôles
				// Remapping "adoucissant" la latitude, par ex. utiliser une fonction sigmoïde ou une racine carrée pour "étirer" plus vers les pôles :
				v = glm::pow(v, 0.75f); // adoucit la compression, à ajuster selon l'effet désiré

				vertexUV.emplace_back(u, v);
			}
		}
	};

	// ------------------------------------------------------------------------------------------------
	// Box Generator
	class ProceduralBoxMesh : public MeshGenerator
	{
	public:
		glm::vec3 size;
		int resolutionX, resolutionY, resolutionZ;
		glm::vec3 rotationAxis;

		ProceduralBoxMesh(const MeshSettings& meshSettings, const GenerationSettings& genSettings)
			: MeshGenerator(meshSettings.position, PrimitiveType::BOX),
			size(meshSettings.size),
			resolutionX(meshSettings.resolutionX),
			resolutionY(meshSettings.resolutionY),
			resolutionZ(meshSettings.resolutionZ),
			rotationAxis(meshSettings.rotationAxis)
		{
			this->generate(genSettings);
		}
		void generate(const GenerationSettings& genSettings)
		{
			this->generateVertexPosition();
			this->generateIndice();
			if (genSettings.generateNormals) this->generateVertexNormals();
			if (genSettings.generateUV) this->generateCubeUVs();
			if (genSettings.generateVBOBuffer && genSettings.generateNormals && genSettings.generateUV) this->generateVBOBuffer();
		}

	private:
		void generateVertexPosition()
		{
			//On accepte la redondance des sommets et des arretes. Ce serais un problemes sur un grand nombre d'instanciations ce qui n'est pas notre cas
			//De plus nos vecteur normaux seront pas interpollés et donc on aurra un rendu lisse
			//De plus le calcul des indices / normals sera plus facille à implémenter
			//Si jamais nous aurrions voulu opti je pourrais construire un tableau de sommets indexés en fonction de leur position logique dans un volume(i, j, k) 
			// et générer une map 3D logique de sommets.
			// Puis, générer les indices par face à partir de cette grille partagée
			this->vertexPositions.clear();

			glm::mat3 rotationMatrix = glm::mat3_cast(glm::rotation(glm::vec3(0, 0, 1), glm::normalize(rotationAxis)));

			float halfX = size.x / 2.0f;
			float halfY = size.y / 2.0f;
			float halfZ = size.z / 2.0f;

			// HAUT
			for (int i = 0; i <= resolutionY; i++) {
				float y = size.y * ((float)i / resolutionY) - halfY;
				for (int j = 0; j <= resolutionX; j++) {
					float x = size.x * ((float)j / resolutionX) - halfX;
					glm::vec3 p(x, y, halfZ);
					this->vertexPositions.emplace_back(cleanVec3(rotationMatrix * (p + meshCenter)));
				}
			}
			// BAS
			for (int i = 0; i <= resolutionY; i++) {
				float y = size.y * ((float)i / resolutionY) - halfY;
				for (int j = 0; j <= resolutionX; j++) {
					float x = size.x * ((float)j / resolutionX) - halfX;
					glm::vec3 p(x, y, -halfZ);
					this->vertexPositions.emplace_back(cleanVec3(rotationMatrix * (p + meshCenter)));
				}
			}

			// DEVANT
			for (int i = 0; i <= resolutionX; i++) {
				float x = size.x * ((float)i / resolutionX) - halfX;
				for (int j = 0; j <= resolutionZ; j++) {
					float z = size.z * ((float)j / resolutionZ) - halfZ;
					glm::vec3 p(x, halfY, z);
					this->vertexPositions.emplace_back(cleanVec3(rotationMatrix * (p + meshCenter)));
				}
			}
			// DERRIERE
			for (int i = 0; i <= resolutionX; i++) {
				float x = size.x * ((float)i / resolutionX) - halfX;
				for (int j = 0; j <= resolutionZ; j++) {
					float z = size.z * ((float)j / resolutionZ) - halfZ;
					glm::vec3 p(x, -halfY, z);
					this->vertexPositions.emplace_back(cleanVec3(rotationMatrix * (p + meshCenter)));
				}
			}

			// DROITE
			for (int i = 0; i <= resolutionY; i++) {
				float y = size.y * ((float)i / resolutionY) - halfY;
				for (int j = 0; j <= resolutionZ; j++) {
					float z = size.z * ((float)j / resolutionZ) - halfZ;
					glm::vec3 p(halfX, y, z);
					this->vertexPositions.emplace_back(cleanVec3(rotationMatrix * (p + meshCenter)));
				}
			}
			// GAUCHE
			for (int i = 0; i <= resolutionY; i++) {
				float y = size.y * ((float)i / resolutionY) - halfY;
				for (int j = 0; j <= resolutionZ; j++) {
					float z = size.z * ((float)j / resolutionZ) - halfZ;
					glm::vec3 p(-halfX, y, z);
					this->vertexPositions.emplace_back(cleanVec3(rotationMatrix * (p + meshCenter)));
				}
			}
		}
		void generateIndice() {
			EBOBuffer.clear();

			int faceIndex = 0;

			auto addQuadIndices = [&](int startIndex, int rows, int cols) {
				for (int i = 0; i < rows; i++) {
					for (int j = 0; j < cols; j++) {
						int topLeft = startIndex + i * (cols + 1) + j;
						int bottomLeft = topLeft + (cols + 1);
						int topRight = topLeft + 1;
						int bottomRight = bottomLeft + 1;

						// Triangle 1
						EBOBuffer.emplace_back(topLeft);
						EBOBuffer.emplace_back(bottomLeft);
						EBOBuffer.emplace_back(topRight);

						// Triangle 2
						EBOBuffer.emplace_back(topRight);
						EBOBuffer.emplace_back(bottomLeft);
						EBOBuffer.emplace_back(bottomRight);
					}
				}
				};

			int faceVertexCount;

			// HAUT
			faceVertexCount = (resolutionX + 1) * (resolutionY + 1);
			addQuadIndices(faceIndex, resolutionY, resolutionX);
			faceIndex += faceVertexCount;

			// BAS
			addQuadIndices(faceIndex, resolutionY, resolutionX);
			faceIndex += faceVertexCount;

			// DEVANT
			faceVertexCount = (resolutionX + 1) * (resolutionZ + 1);
			addQuadIndices(faceIndex, resolutionZ, resolutionX);
			faceIndex += faceVertexCount;

			// DERRIÈRE
			addQuadIndices(faceIndex, resolutionZ, resolutionX);
			faceIndex += faceVertexCount;

			// DROITE
			faceVertexCount = (resolutionY + 1) * (resolutionZ + 1);
			addQuadIndices(faceIndex, resolutionZ, resolutionY);
			faceIndex += faceVertexCount;

			// GAUCHE
			addQuadIndices(faceIndex, resolutionZ, resolutionY);
		}
		void generateCubeUVs() {
			vertexUV.clear();
			vertexUV.reserve(vertexPositions.size());

			auto addFaceUVs = [&](int resU, int resV) {
				for (int v = 0; v <= resV; ++v) {
					for (int u = 0; u <= resU; ++u) {
						float uCoord = static_cast<float>(u) / resU;
						float vCoord = static_cast<float>(v) / resV;
						vertexUV.emplace_back(uCoord, vCoord);
					}
				}
				};

			// Ordre des faces identique à generateVertexPosition
			// Haut (XY)
			addFaceUVs(resolutionX, resolutionY);

			// Bas (XY)
			addFaceUVs(resolutionX, resolutionY);

			// Devant (XZ)
			addFaceUVs(resolutionX, resolutionZ);

			// Derrière (XZ)
			addFaceUVs(resolutionX, resolutionZ);

			// Droite (YZ)
			addFaceUVs(resolutionY, resolutionZ);

			// Gauche (YZ)
			addFaceUVs(resolutionY, resolutionZ);
		}
	};

	// ------------------------------------------------------------------------------------------------
	// Cylinder Generator (IA Generated - won't take credit for this)
	class ProceduralCylinderMesh : public MeshGenerator
	{
	public:
		float radius;
		float height;
		int resolutionRadial;   // nombre de subdivisions autour
		int resolutionHeight;   // nombre de subdivisions en hauteur
		glm::vec3 rotationAxis;

		ProceduralCylinderMesh(const MeshSettings& meshSettings, const GenerationSettings& genSettings)
			: MeshGenerator(meshSettings.position, PrimitiveType::CYLINDER),
			radius(meshSettings.radius),
			height(meshSettings.height),
			resolutionRadial(meshSettings.resolutionX),
			resolutionHeight(meshSettings.resolutionY),
			rotationAxis(meshSettings.rotationAxis)
		{
			this->generate(genSettings);
		}

	private:
		void generate(const GenerationSettings& genSettings)
		{
			generateVertexPosition();
			generateIndice();
			if (genSettings.generateNormals) generateVertexNormals();
			if (genSettings.generateUV) generateCylindricalUVs();
			if (genSettings.generateVBOBuffer && genSettings.generateNormals && genSettings.generateUV)
				generateVBOBuffer();
		}
		void generateVertexPosition()
		{
			vertexPositions.clear();
			glm::mat3 rotationMatrix = glm::mat3_cast(glm::rotation(glm::vec3(0, 0, 1), glm::normalize(rotationAxis)));

			float halfHeight = height / 2.0f;
			float angleStep = 2.0f * glm::pi<float>() / resolutionRadial;

			// Bas (centre)
			glm::vec3 baseCenter = cleanVec3(rotationMatrix * glm::vec3(0, -halfHeight, 0) + meshCenter);
			vertexPositions.push_back(baseCenter);

			for (int i = 0; i <= resolutionRadial; ++i) {
				float angle = i * angleStep;
				glm::vec3 p = glm::vec3(std::cos(angle) * radius, -halfHeight, std::sin(angle) * radius);
				vertexPositions.push_back(cleanVec3(rotationMatrix * p + meshCenter));
			}

			// Haut (centre)
			glm::vec3 topCenter = cleanVec3(rotationMatrix * glm::vec3(0, halfHeight, 0) + meshCenter);
			vertexPositions.push_back(topCenter);

			for (int i = 0; i <= resolutionRadial; ++i) {
				float angle = i * angleStep;
				glm::vec3 p = glm::vec3(std::cos(angle) * radius, halfHeight, std::sin(angle) * radius);
				vertexPositions.push_back(cleanVec3(rotationMatrix * p + meshCenter));
			}

			// Paroi
			for (int y = 0; y <= resolutionHeight; ++y) {
				float h = -halfHeight + height * y / resolutionHeight;
				for (int i = 0; i <= resolutionRadial; ++i) {
					float angle = i * angleStep;
					glm::vec3 p = glm::vec3(std::cos(angle) * radius, h, std::sin(angle) * radius);
					vertexPositions.push_back(cleanVec3(rotationMatrix * p + meshCenter));
				}
			}
		}
		void generateIndice()
		{
			EBOBuffer.clear();
			int baseBottom = 0;
			int baseTop = resolutionRadial + 2;
			int baseSide = baseTop + resolutionRadial + 2;

			// --- 1. Disque bas
			for (int i = 0; i < resolutionRadial; ++i) {
				EBOBuffer.push_back(baseBottom);             // centre
				EBOBuffer.push_back(baseBottom + i + 1);
				EBOBuffer.push_back(baseBottom + i + 2);
			}

			// --- 2. Disque haut
			for (int i = 0; i < resolutionRadial; ++i) {
				EBOBuffer.push_back(baseTop);               // centre
				EBOBuffer.push_back(baseTop + i + 2);
				EBOBuffer.push_back(baseTop + i + 1);
			}

			// --- 3. Paroi
			int vertsPerRing = resolutionRadial + 1;
			for (int y = 0; y < resolutionHeight; ++y) {
				for (int i = 0; i < resolutionRadial; ++i) {
					int i0 = baseSide + y * vertsPerRing + i;
					int i1 = i0 + 1;
					int i2 = i0 + vertsPerRing;
					int i3 = i2 + 1;

					EBOBuffer.push_back(i0);
					EBOBuffer.push_back(i2);
					EBOBuffer.push_back(i1);

					EBOBuffer.push_back(i1);
					EBOBuffer.push_back(i2);
					EBOBuffer.push_back(i3);
				}
			}
		}
		void generateCylindricalUVs()
		{
			vertexUV.clear();
			vertexUV.reserve(vertexPositions.size());

			glm::mat3 rotationMatrix = glm::mat3_cast(glm::rotation(glm::vec3(0, 1, 0), glm::normalize(rotationAxis)));
			glm::mat3 inverseRotation = glm::transpose(rotationMatrix); // matrice inverse car rotation orthonormale

			for (const glm::vec3& pos : vertexPositions) {
				glm::vec3 local = inverseRotation * (pos - meshCenter); // back to local Y-up space
				glm::vec2 uv;

				// bas ou haut : on projette en plan (XZ)
				if (std::abs(local.y + height / 2.0f) < 1e-5f || std::abs(local.y - height / 2.0f) < 1e-5f) {
					uv.x = 0.5f + local.x / (2.0f * radius);
					uv.y = 0.5f + local.z / (2.0f * radius);
				}
				// paroi : mapping cylindrique
				else {
					float angle = atan2(local.z, local.x); // autour de Y
					uv.x = (angle + glm::pi<float>()) / (2.0f * glm::pi<float>()); // U entre 0-1
					uv.y = (local.y + height / 2.0f) / height; // V entre 0-1
				}
				vertexUV.emplace_back(uv);
			}
		}
	};

	// ------------------------------------------------------------------------------------------------
	// Cone Generator (IA Generated - won't take credit for this)
	class ProceduralConeMesh : public MeshGenerator
	{
	public:
		float radius;
		float height;
		int resolutionX;
		int resolutionY;
		glm::vec3 rotationAxis;

		ProceduralConeMesh(const MeshSettings& meshSettings, const GenerationSettings& genSettings)
			: MeshGenerator(meshSettings.position, PrimitiveType::CONE),
			radius(meshSettings.radius),
			height(meshSettings.height),
			resolutionX(meshSettings.resolutionX),
			resolutionY(meshSettings.resolutionY),
			rotationAxis(meshSettings.rotationAxis)
		{
			this->generate(genSettings);
		}

		void generate(const GenerationSettings genSettings)
		{
			generateVertexPosition();
			generateIndice();
			if (genSettings.generateNormals) generateVertexNormals();
			if (genSettings.generateUV) generateCylindricalUVs();
			if (genSettings.generateVBOBuffer && genSettings.generateNormals && genSettings.generateUV)
				generateVBOBuffer();
		}

	private:
		void generateVertexPosition()
		{
			vertexPositions.clear();

			glm::mat3 rotationMatrix = glm::mat3_cast(glm::rotation(glm::vec3(0, 0, 1), glm::normalize(rotationAxis)));

			glm::vec3 apex = glm::vec3(0, 0, height / 2.0f);
			glm::vec3 baseCenter = glm::vec3(0, 0, -height / 2.0f);
			apex = cleanVec3(rotationMatrix * apex + meshCenter);
			baseCenter = cleanVec3(rotationMatrix * baseCenter + meshCenter);

			vertexPositions.emplace_back(apex);        // 0 - sommet
			vertexPositions.emplace_back(baseCenter);  // 1 - centre base

			for (int i = 0; i <= resolutionX; ++i)
			{
				float angle = 2.0f * glm::pi<float>() * (float)i / resolutionX;
				float x = radius * cos(angle);
				float y = radius * sin(angle);
				glm::vec3 p = glm::vec3(x, y, -height / 2.0f);
				vertexPositions.emplace_back(cleanVec3(rotationMatrix * p + meshCenter));
			}
		}
		void generateIndice()
		{
			EBOBuffer.clear();

			// Faces latérales
			for (int i = 2; i < 2 + resolutionX; ++i)
			{
				int next = (i + 1 <= resolutionX + 1) ? i + 1 : 2;
				EBOBuffer.emplace_back(0); // sommet
				EBOBuffer.emplace_back(i);
				EBOBuffer.emplace_back(next);
			}

			// Base
			for (int i = 2; i < 2 + resolutionX; ++i)
			{
				int next = (i + 1 <= resolutionX + 1) ? i + 1 : 2;
				EBOBuffer.emplace_back(1); // centre base
				EBOBuffer.emplace_back(next);
				EBOBuffer.emplace_back(i);
			}
		}
		void generateCylindricalUVs()
		{
			vertexUV.clear();
			vertexUV.reserve(vertexPositions.size());

			int baseStart = 2; // vertex index du 1er point de la base
			int baseCount = resolutionX + 1;

			// UV sommet
			vertexUV.emplace_back(0.5f, 1.0f); // u quelconque, v = haut

			// UV centre base
			vertexUV.emplace_back(0.5f, 0.0f); // u quelconque, v = bas

			// UV base (cercle)
			for (int i = 0; i < baseCount; ++i)
			{
				float u = (float)i / resolutionX;
				vertexUV.emplace_back(u, 0.0f); // V = 0 pour la base
			}
		}
	};

	// ------------------------------------------------------------------------------------------------
	// Disk Plane Generator
	class ProceduralDiskMesh : public MeshGenerator {
	public:
		float radius;
		int resolutionRadial;
		glm::vec3 rotationAxis;

		ProceduralDiskMesh(const MeshSettings& meshSettings, const GenerationSettings& genSettings)
			: MeshGenerator(meshSettings.position, PrimitiveType::DISK),
			radius(meshSettings.radius),
			resolutionRadial(meshSettings.resolutionX),
			rotationAxis(meshSettings.rotationAxis)
		{
			this->generate(genSettings);
		}

		void generate(const GenerationSettings genSettings) {
			generateVertexPosition();
			generateIndice();
			if (genSettings.generateNormals) generateVertexNormals();
			if (genSettings.generateUV) generateDiskUVs();
			if (genSettings.generateVBOBuffer && genSettings.generateNormals && genSettings.generateUV)
				generateVBOBuffer();
		}

	private:
		void generateVertexPosition() {
			vertexPositions.clear();
			glm::mat3 rotationMatrix = glm::mat3_cast(glm::rotation(glm::vec3(0.0f, 0.0f, 1.0f), glm::normalize(this->rotationAxis)));

			// Centre
			glm::vec3 baseCenter = meshCenter;
			vertexPositions.push_back(baseCenter);

			//Cercle
			float angleStep = 2.0f * glm::pi<float>() / resolutionRadial;
			for (int i = 0; i <= resolutionRadial; ++i) {
				float angle = i * angleStep;
				glm::vec3 p = glm::vec3(std::cos(angle) * radius, std::sin(angle) * radius, 0);
				vertexPositions.push_back(cleanVec3(rotationMatrix * p + meshCenter));
			}
		}
		void generateIndice() {
			EBOBuffer.clear();
			int baseBottom = 0;

			for (int i = 0; i < resolutionRadial; ++i) {
				int center = baseBottom;
				int next = baseBottom + i + 2;
				int current = baseBottom + i + 1;

				// Si on est au dernier triangle, on revient au début
				if (i == resolutionRadial - 1) {
					next = baseBottom + 1;
				}

				EBOBuffer.push_back(center);
				EBOBuffer.push_back(current);
				EBOBuffer.push_back(next);
			}
		}
		void generateDiskUVs() {
			vertexUV.clear();

			// Centre du disque au centre de la texture (0.5, 0.5)
			vertexUV.push_back(glm::vec2(0.5f, 0.5f));

			float angleStep = 2.0f * glm::pi<float>() / resolutionRadial;

			// Périphérie du disque coordonnées UV circulaires
			for (int i = 0; i <= resolutionRadial; ++i) {
				float angle = i * angleStep;
				float u = 0.5f + 0.5f * std::cos(angle);
				float v = 0.5f + 0.5f * std::sin(angle);
				vertexUV.push_back(glm::vec2(u, v));
			}
		}
	};











	// ------------------------------------------------------------------------------------------------
	// FACTORY BUILDER
	class MeshFactory
	{
	public:
		static std::unique_ptr<MeshGenerator> createMesh(const PrimitiveType& meshType, const MeshSettings& meshSettings, const GenerationSettings& genSettings = { true, true, true })
		{
			switch (meshType)
			{
			case PrimitiveType::SPHERE:
				return std::make_unique<ProceduralSphereMesh>(meshSettings, genSettings);

			case PrimitiveType::BOX:
				return std::make_unique<ProceduralBoxMesh>(meshSettings, genSettings);

			case PrimitiveType::CYLINDER:
				return std::make_unique<ProceduralCylinderMesh>(meshSettings, genSettings);

			case PrimitiveType::CONE:
				return std::make_unique<ProceduralConeMesh>(meshSettings, genSettings);

			case PrimitiveType::DISK:
				return std::make_unique<ProceduralDiskMesh>(meshSettings, genSettings);

			default:
				std::cerr << "Unknown primitive type." << std::endl;
				return nullptr;
			}
		}
	};
}
#endif