#ifndef DRAP_H
#define DRAP_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <memory> // Pour std::shared_ptr
#include "point.h"
#include "triangle.h"

namespace Drap
{
	struct DrapSettings
	{
		float sizeX = 10.8f;
		float sizeY = 10.8f;
		unsigned int resolutionX = 5;
		unsigned int resolutionY = 5;
		GLfloat startingHeight = 0.5f;
		float gapPointStart = sizeX / resolutionX;
		float floorHeight = -10.0f;
		float mass = 1.0f;
	};


	class Drap
	{
	public:
		int nb_point_x;
		int nb_point_y;
		float gapPointInit;
		std::vector<std::vector<std::shared_ptr<Point>>> ListeDesPointsDansLeDrap; // Tableau [x][y] des ptr des points
		std::vector<std::shared_ptr<Point>> ListeDesPointsDansLeDrapFlattened;  
		std::vector<std::shared_ptr<Triangle>> ListeDesTrianglesDansLeDrap;

		std::vector<GLfloat> VBOBuffer;
		std::vector<unsigned int> EAOIndices;
		
		std::vector<GLfloat> NormalOfEachPoint;


		// Constructeur de Drap
		Drap(DrapSettings drapSettings)
			: nb_point_x(drapSettings.resolutionX), nb_point_y(drapSettings.resolutionY), gapPointInit(drapSettings.gapPointStart)
		{
			for (int i = 0; i < nb_point_x; i++) {
				std::vector<std::shared_ptr<Point>> TempVector = {};
				for (int j = 0; j < nb_point_y; j++) {
					TempVector.emplace_back(std::make_shared<Point>(
						i * gapPointInit,
						j * gapPointInit,
						drapSettings.startingHeight
					));
				}
				ListeDesPointsDansLeDrap.emplace_back(TempVector);
			}

			generateBufferDrapVertex();
			generateEAOindices();
			generateNeighborPoints();
			flatteningVectorPoint();
			generateTriangle();
			generateLinkPointTriangle();
		}

		// Génération du buffer de VBO
		void generateBufferDrapVertex() {
			VBOBuffer.clear();
			for (auto& q : ListeDesPointsDansLeDrap)
			{
				for (auto& p : q) 
				{
					//VertexPosVec
					VBOBuffer.emplace_back(p->vecPos.x);
					VBOBuffer.emplace_back(p->vecPos.y);
					VBOBuffer.emplace_back(p->vecPos.z);
					//NormalVec
					VBOBuffer.emplace_back(p->vecNormal.x);
					VBOBuffer.emplace_back(p->vecNormal.y);
					VBOBuffer.emplace_back(p->vecNormal.z);
					//TextureVec
					VBOBuffer.emplace_back(p->vecPos.x * (1.0f / gapPointInit / (nb_point_x - 1)));
					VBOBuffer.emplace_back(p->vecPos.y * (1.0f / gapPointInit / (nb_point_y - 1)));
				}
			}
		}

		// Génération des indices pour les éléments du tableau
		void generateEAOindices() {
			EAOIndices.clear();
			for (int i = 0; i < nb_point_x - 1; i++) {
				for (int j = 0; j < nb_point_y - 1; j++) {
					EAOIndices.emplace_back(i + j * nb_point_x);                    //Triangle 1
					EAOIndices.emplace_back(i + 1 + nb_point_x + j * nb_point_x);   // Inversion de l'ordre des 2 points pour le calcul des normals
					EAOIndices.emplace_back(i + 1 + j * nb_point_x);                 

					EAOIndices.emplace_back(i + j * nb_point_x);                    //Triangle 1-bis (complementaire/formant un carré)
					EAOIndices.emplace_back(i + nb_point_x + j * nb_point_x);
					EAOIndices.emplace_back(i + 1 + nb_point_x + j * nb_point_x);
				}
			}
		}

		void generateNeighborPoints()
		{
			for (int i = 0; i < nb_point_x; ++i) {
				for (int j = 0; j < nb_point_y; ++j) {
					auto& center = ListeDesPointsDansLeDrap[i][j];

					// Gauche
					if (i > 0)
						center->listVoisin.emplace_back(ListeDesPointsDansLeDrap[i - 1][j]);

					// Droite
					if (i < nb_point_x - 1)
						center->listVoisin.emplace_back(ListeDesPointsDansLeDrap[i + 1][j]);

					// Bas
					if (j > 0)
						center->listVoisin.emplace_back(ListeDesPointsDansLeDrap[i][j - 1]);

					// Haut
					if (j < nb_point_y - 1)
						center->listVoisin.emplace_back(ListeDesPointsDansLeDrap[i][j + 1]);
				}
			}
		}

		// Affichage du buffer VBO
		void printVBOBuffer() {
			std::cout << "\nPrinting VBO Buffer" << std::endl;
			for (size_t h = 0; h < VBOBuffer.size(); h += 8) {
				std::cout << "Vertex Pos: (" << VBOBuffer[h] << "/" << VBOBuffer[h + 1] << "/" << VBOBuffer[h + 2] << ")" << std::endl;
				std::cout << "  Normal:(" << VBOBuffer[h + 3] << "/" << VBOBuffer[h + 4] << "/" << VBOBuffer[h + 5] << ")" << std::endl;
				std::cout << "  Texture:(" << VBOBuffer[h + 6] << "/" << VBOBuffer[h + 7] << ")" << std::endl;
			}
			std::cout << "\n" << std::endl;
		}

		// Affichage du buffer des indices EAO
		void printEAOBuffer() {
			std::cout << "\nPrinting EAO Buffer" << std::endl;
			for (size_t h = 0; h < EAOIndices.size(); h += 3) {
				std::cout << "(" << EAOIndices[h] << "-" << EAOIndices[h + 1] << "-" << EAOIndices[h + 2] << ")" << std::endl;
			}
			std::cout << "\n" << std::endl;
		}

		// Affichage de la liste des points et leurs voisins
		void printListPoints() {
			std::cout << "\n\nPrinting List of Points" << std::endl;
			std::cout << "\nNb Points: " << ListeDesPointsDansLeDrap.size() * ListeDesPointsDansLeDrap[0].size() << std::endl;
			std::cout << "-----------" << std::endl;
			for (auto& q : ListeDesPointsDansLeDrap)
			{
				for (auto& p : q) {
					p->printAllInfo();
					std::cout << std::endl;
				}
			} 
		}

		// Affichage de la liste des points flattened
		void printListPointsFlattened() {
			std::cout << "\n\nPrinting List of Points FLATTENED" << std::endl;
			std::cout << "\nNb Points: " << ListeDesPointsDansLeDrapFlattened.size() << std::endl;
			std::cout << "-----------" << std::endl;
			for (auto& q : ListeDesPointsDansLeDrapFlattened)
			{
				q->printPoint();
				std::cout << std::endl;
			}
		}

		void printListTriangle()
		{
			std::cout << "\n\nPrinting List of Triangle" << std::endl;
			std::cout << "\nNb Triangle: " << ListeDesTrianglesDansLeDrap.size() << std::endl;
			std::cout << "-----------" << std::endl;
			for (auto& t : ListeDesTrianglesDansLeDrap)
			{
				t->printTriangle();
			}
		}

		void updateNormal()
		{
			for (auto& t : ListeDesTrianglesDansLeDrap)
			{
				t->updateNormal();
			}
			for (auto& q : ListeDesPointsDansLeDrap)
			{
				for (auto& p : q) {
					p->updateNormal();
				}
			}
		}

		void downloadNewVertexPosition(Drap temp_drap)
		{
			for (int i = 0; i < ListeDesPointsDansLeDrapFlattened.size(); i++)
			{
				ListeDesPointsDansLeDrapFlattened[i]->vecPos = temp_drap.ListeDesPointsDansLeDrapFlattened[i]->vecPos;
			}
		}

		void downloadAncienneVertexPosition(Drap temp_drap)
		{
			for (int i = 0; i < ListeDesPointsDansLeDrapFlattened.size(); i++)
			{
				ListeDesPointsDansLeDrapFlattened[i]->vecPosAncienne = temp_drap.ListeDesPointsDansLeDrapFlattened[i]->vecPosAncienne;
			}
		}

		void updateAncienneVertexPosition()
		{
			for (int i = 0; i < ListeDesPointsDansLeDrapFlattened.size(); i++)
			{
				ListeDesPointsDansLeDrapFlattened[i]->vecPosAncienne = ListeDesPointsDansLeDrapFlattened[i]->vecPos;
			}
		}
		

	private:

		void flatteningVectorPoint()
		{
			for (auto& q : ListeDesPointsDansLeDrap)
			{
				for (auto& p : q) {
					ListeDesPointsDansLeDrapFlattened.emplace_back(p);
				}
			}
		}

		void generateTriangle()
		{
			//Verifions tout de meme que EAOIndices.size() soit un multiple de 3
			if (EAOIndices.size() % 3 == 0)
			{
				for (int i = 0; i < EAOIndices.size(); i = i + 3) // 3 par 3 pour chaque triangle
				{
					//on obtient les indices des formant les triangle
					//ces indices sont ceux des Points dans ListeDesPointsDansLeDrapFlattened
					auto& p0 = ListeDesPointsDansLeDrapFlattened[EAOIndices[i]];
					auto& p1 = ListeDesPointsDansLeDrapFlattened[EAOIndices[i + 1]];
					auto& p2 = ListeDesPointsDansLeDrapFlattened[EAOIndices[i + 2]];

					ListeDesTrianglesDansLeDrap.emplace_back(std::make_shared<Triangle>(p0, p1, p2));
				}
			}
			else { std::cout << "ERREUR: calculVertexNormal: EAOIndices n'est pas un multiple de 3" << std::endl; }
		}

		void generateLinkPointTriangle()
		{
			for (auto& t : ListeDesTrianglesDansLeDrap)
			{
				t->Sommet0->linkedSurfaces.emplace_back(t);
				t->Sommet1->linkedSurfaces.emplace_back(t);
				t->Sommet2->linkedSurfaces.emplace_back(t);
			}
			for (auto& q : ListeDesPointsDansLeDrap)
			{
				for (auto& p : q) {
					p->updateNormal();
				}
			}
		}

	};
}


#endif