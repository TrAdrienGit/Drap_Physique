#ifndef PHYSIC_H
#define PHYSIC_H

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include "vertex.h"
#include "tissu.h"
#include "glencapsulation.h"
#include "mesh_generator.h"

// ----------------------------------------------------------------------------------------------------


namespace Physics
{
	// ------------------------------------------------------------------------------------------------
	// Settings structs
	// Physique parametres
	struct GravitySettings {
		float amplitude = 9.81f * 0.005f;
		glm::vec3 directionVector = glm::vec3(0.0f, 0.0f, -1.0f);
	};
	struct TensionSettings {
		float longeurVideRessort = 0.1f;
		float force = 500.0f;
		float maxTensionForce = 10.0f;
		float damping = 200000.0f;
	};
	struct CollisionSettings {
		float antiClippingGap = 0.1f;
		float stiffness = 50000.0f;
	};
	struct TemporalSettings {
		float dt = 1.0f / 120.0f;
		int substeps = 20; // Les substeps sont le nombre de fois que l'on fait une simultion physique par frame | on fait des simulations plus petites, plus frequente, donc plus précices
	};
	struct PhysicsSettings {
		GravitySettings gravitySettings;
		TensionSettings tensionSettings;
		CollisionSettings collisionSettings;
		TemporalSettings temporalSettings;
	};

	// ------------------------------------------------------------------------------------------------
	// Tension Force 
	glm::vec3 tensionF(
		const glm::vec3& point1,
		const glm::vec3& point2,
		const glm::vec3& vel1,
		const glm::vec3& vel2,
		float k,
		float l0,
		float damping)
	{
		glm::vec3 direction = point2 - point1;

		float distance = glm::length(direction);

		// Éviter la division par zéro
		if (distance < 1e-6f) {
			return glm::vec3(0.0f);
		}

		// Force de ressort (Hooke)
		float displacement = distance - l0;
		glm::vec3 forceSpring = k * displacement * direction;

		// Amortissement (damping)
		glm::vec3 relativeVelocity = vel2 - vel1;
		float dampingForce = glm::dot(relativeVelocity, direction);
		glm::vec3 forceDamping = damping * dampingForce * direction;

		return forceSpring + forceDamping;

	}

	glm::vec3 calculerForceTensionDepuisPointsFixes(
		std::shared_ptr<Vertex>& pointMobile,
		std::vector<std::weak_ptr<Vertex>>& pointsFixesVoisins,
		float k,
		float l0,
		float damping,
		float maxTensionForce
	)
	{
		glm::vec3 forceTotale(0.0f);  //C'est une acceleration


		for (auto& voisin : pointsFixesVoisins)
		{
			if (auto voisinp = voisin.lock())
			{
				// On vérifie que le voisin existe
				if (voisinp != nullptr)
				{
					glm::vec3 force = tensionF(
						pointMobile->position,
						voisinp->position,
						pointMobile->calculVitesse(),
						voisinp->calculVitesse(),
						k,
						l0,
						damping);
					if (glm::length(force) > maxTensionForce) {
						force = glm::normalize(force) * maxTensionForce;
					}
					forceTotale += force;
				}
			}
		}

		

		return forceTotale;
	}

	// ------------------------------------------------------------------------------------------------
	// Sphere Collision
	void applySphereCollisionResponse(
		auto& listPoints,
		const glm::vec3& sphereCenter,
		float sphereRadius,
		float antiClippingGap,
		float stiffness)
	{
		for (int i = 0; i < listPoints.size(); i++)
		{
			for (int j = 0; j < listPoints[i].size(); j++)
			{
				glm::vec3 direction = listPoints[i][j]->position - sphereCenter;
				float distance = glm::length(direction);

				if (distance <= sphereRadius + antiClippingGap) {
					float penetration = sphereRadius + antiClippingGap - distance;
					glm::vec3 normal = (distance > 0.0f) ? glm::normalize(direction) : glm::vec3(0, 1, 0);
					glm::vec3 reaction_force = normal * penetration * stiffness;
					listPoints[i][j]->acceleration += reaction_force;
				}
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	// Box Collision
	float sdfBox(const glm::vec3& p, const glm::vec3& halfSize)
	{
		glm::vec3 d = glm::abs(p) - halfSize;
		return glm::length(glm::max(d, glm::vec3(0.0f))) + std::min(glm::max(d.x, glm::max(d.y, d.z)), 0.0f);
	}

	void applyBoxCollisionResponse_SDF(
		auto& listPoints,
		const glm::vec3& boxCenter,
		glm::vec3 boxSize,
		const glm::vec3& rotationAxis,
		float stiffness)
	{
		glm::mat3 rotationMatrix = glm::mat3(1.0f);
		if (glm::length(rotationAxis) > 1e-6f) {
			rotationMatrix = glm::mat3_cast(glm::rotation(glm::vec3(0, 0, 1), glm::normalize(rotationAxis)));
		}

		glm::mat3 invRotation = glm::transpose(rotationMatrix);
		glm::vec3 halfSize = (boxSize + glm::vec3(0.1f, 0.1f, 0.03f) * 6.0f) * 0.5f;     //// ATTENTION NOMBRE MAGIQUE ICIIII

		for (int i = 0; i < listPoints.size(); i++)
		{
			for (auto& point : listPoints[i])
			{
				glm::vec3 localPos = invRotation * (point->position - boxCenter);
				float dist = sdfBox(localPos, halfSize);

				if (dist < 0.0f) // Point à l’intérieur
				{
					// Approximation du gradient (normal) via dérivée centrale
					float h = 0.001f;
					glm::vec3 normal = glm::normalize(glm::vec3(
						sdfBox(localPos + glm::vec3(h, 0, 0), halfSize) - sdfBox(localPos - glm::vec3(h, 0, 0), halfSize),
						sdfBox(localPos + glm::vec3(0, h, 0), halfSize) - sdfBox(localPos - glm::vec3(0, h, 0), halfSize),
						sdfBox(localPos + glm::vec3(0, 0, h), halfSize) - sdfBox(localPos - glm::vec3(0, 0, h), halfSize)
					));

					glm::vec3 worldNormal = rotationMatrix * normal;
					point->acceleration += worldNormal * (-dist) * stiffness;
				}
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	// Cylinder Collision

	// ------------------------------------------------------------------------------------------------
	// Cone Collision

	




	// ------------------------------------------------------------------------------------------------
	// ------------------------------------------------------------------------------------------------
	// Main Calculation
	void applicationForces(Tissu::Tissu& LeTissu, Tissu::Tissu temp_Tissu, const std::vector<MeshGenerator::MeshGenerator*>& listObj, const PhysicsSettings& physicsSettings, const Tissu::TissuSettings& tissuSettings)
	{
		//Verification que nos tissu possedent le meme nombre de points
		if (LeTissu.listVertices.size() == temp_Tissu.listVertices.size())
		{
			// -------------------------------------------------------------------------------------
			// Acceleraction remise à zéro à chaque frame
			for (int i = 0; i < LeTissu.listVertices.size(); i++) {
				for (int j = 0; j < LeTissu.listVertices[i].size(); j++) {
					LeTissu.listVertices[i][j]->acceleration = glm::vec3(0.0f);
				}
			}

			// -------------------------------------------------------------------------------------
			//Mise à jours des position de points de temp_tissu avec ceux du vrai tissu par copie
			temp_Tissu.downloadNewVertexPosition(LeTissu);
			temp_Tissu.downloadAncienneVertexPosition(LeTissu);

			// -------------------------------------------------------------------------------------
			// Gravité
			for (int i = 0; i < LeTissu.listVertices.size(); i++) {
				for (int j = 0; j < LeTissu.listVertices[i].size(); j++) {
					if (LeTissu.listVertices[i][j]->position.z >= tissuSettings.floorHeight &&
						LeTissu.listVertices[i][j]->isAffectedGravity
						&& !LeTissu.listVertices[i][j]->isFixed)
					{
						LeTissu.listVertices[i][j]->acceleration += physicsSettings.gravitySettings.directionVector * physicsSettings.gravitySettings.amplitude * (1.0f / tissuSettings.mass);
					}
				}
			}
			// Tension
			for (int i = 0; i < LeTissu.listVertices.size(); i++) {
				for (int j = 0; j < LeTissu.listVertices[i].size(); j++) {
					if (LeTissu.listVertices[i][j]->isAffectedTension && !LeTissu.listVertices[i][j]->isFixed)
					{
						LeTissu.listVertices[i][j]->acceleration += calculerForceTensionDepuisPointsFixes(LeTissu.listVertices[i][j], LeTissu.listVertices[i][j]->linkedVertices, physicsSettings.tensionSettings.force, physicsSettings.tensionSettings.longeurVideRessort, physicsSettings.tensionSettings.damping, physicsSettings.tensionSettings.maxTensionForce);
					}
				}
			}
			// Collsion
			for (MeshGenerator::MeshGenerator* obj : listObj)
			{
				switch (obj->type)
				{
				default:
					std::cerr << "ERROR: Application Force: Wrong Mesh Type" << std::endl;
					break;

				case MeshGenerator::PrimitiveType::SPHERE:
				{
					auto* sphere_obj = dynamic_cast<MeshGenerator::ProceduralSphereMesh*>(obj);
					if (sphere_obj)
					{
						applySphereCollisionResponse(LeTissu.listVertices, sphere_obj->meshCenter, sphere_obj->radius, physicsSettings.collisionSettings.antiClippingGap, physicsSettings.collisionSettings.stiffness);
					}
					break;
				}

				case MeshGenerator::PrimitiveType::BOX:
				{
					auto* box_obj = dynamic_cast<MeshGenerator::ProceduralBoxMesh*>(obj);
					if (box_obj)
					{
						//applyBoxCollisionResponse(LeTissu.listVertices, box_obj->meshCenter, box_obj->size, box_obj->rotationAxis, antiClippingGap, collision_stiffness);
						applyBoxCollisionResponse_SDF(LeTissu.listVertices, box_obj->meshCenter, box_obj->size, box_obj->rotationAxis, physicsSettings.collisionSettings.stiffness);
					}
					break;
				}
				}
			}

			// -------------------------------------------------------------------------------------
			//Application de l'intégration de Verlet (+pression/+réaliste)
			//x(t + 1) = x(t) + [x(t) - x(t - 1)] + a(t) * dt²               [x(t) - x(t - 1)] c'est la vitesse
			for (int i = 0; i < LeTissu.listVertices.size(); i++) {
				for (int j = 0; j < LeTissu.listVertices[i].size(); j++) {
					auto& point = LeTissu.listVertices[i][j];
					auto& tempPoint = temp_Tissu.listVertices[i][j];

					if (!point->isFixed && point->position.z > tissuSettings.floorHeight)
					{
						tempPoint->position = point->position + point->calculVitesse() + point->acceleration * (physicsSettings.temporalSettings.dt * physicsSettings.temporalSettings.dt) / physicsSettings.temporalSettings.substeps;
					}
					else if (!point->isFixed && point->position.z <= tissuSettings.floorHeight)
					{
						tempPoint->isFixed = true;
						tempPoint->position.z = tissuSettings.floorHeight;
					}
					else
					{
						tempPoint->position = point->position; // point fixe
					}
				}
			}

			// -------------------------------------------------------------------------------------
			// Implementation Double Buffering pour limiter les déformations artificielles causées par le traitement séquentiel
			LeTissu.updateAncienneVertexPosition();
			LeTissu.downloadNewVertexPosition(temp_Tissu);
		}
		else
		{
			std::cout << "ERREUR: Appliquation Physique: Le double buffering n'a pas cree 2 tissus de meme taille" << std::endl;
		}
	}

	// Main Calculation Substep
	void physicCalculationSubsteped(Tissu::Tissu& tissu, const std::vector<MeshGenerator::MeshGenerator*>& listObj, const PhysicsSettings& physicsSettings, const Tissu::TissuSettings& tissuSettings)
	{
		static Tissu::Tissu temp_tissu = Tissu::Tissu(tissuSettings);
		for (int s = 0; s < physicsSettings.temporalSettings.substeps; ++s) {
			applicationForces(tissu, temp_tissu, listObj, physicsSettings, tissuSettings);
		}
	}
}
#endif
