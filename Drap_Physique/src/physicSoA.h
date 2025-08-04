#ifndef PHYSICSOA_H
#define PHYSICSOA_H

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <unordered_map>
#include <cmath>
#include <omp.h>
#include "tissuSoA.h"
#include "glencapsulation.h"
#include "mesh_generator.h"
#include "grabbing.h"

// ----------------------------------------------------------------------------------------------------

namespace Physics
{
	// ------------------------------------------------------------------------------------------------
	// Settings structs
	// Physique parametres
	struct GravitySettings {
		float amplitude = 9.81f * 0.005f;
		glm::vec3 directionVector = glm::vec3(0.0f, 0.0f, -1.0f);
		bool isEnabled = true;
	};
	struct TensionSettings {
		float longeurVideRessortX = 0.1f;
		float longeurVideRessortY = 0.1f;
		float force = 500.0f;
		float maxTensionForce = 10.0f;
		float damping = 200000.0f;
		bool isEnabled = true;
	};
	struct CollisionSettings {
		float antiClippingGap = 0.1f;
		float stiffness = 50000.0f;
		float damping = 20000.0f;
		bool isEnabled = true;
	};
	struct SelfCollisionSettings {
		float minDistance = 0.04f;
		float cellSize = 0.08f;
		bool isEnabled = false;
	};
	struct WindSettings {
		glm::vec3 directionVector = glm::vec3(1.0f, 0.0f, 0.0f);
		float amplitude = 1.0f;
		float oscillationFrequency = 1.0f;
		bool isOscillate = true;
		bool isEnabled = true;
	};
	struct GrabSettings {
		float force = 500.0f;
		float damping = 200000.0f; //amortissement
		bool isEnabled = true;
	};
	struct TemporalSettings {
		float dt = 1.0f / 120.0f;
		int substeps = 20; // Les substeps sont le nombre de fois que l'on fait une simultion physique par frame | on fait des simulations plus petites, plus frequente, donc plus précices
	};
	struct PhysicsSettings {
		GravitySettings gravitySettings;
		TensionSettings tensionSettings;
		CollisionSettings collisionSettings;
		SelfCollisionSettings selfCollisionSettings;
		TemporalSettings temporalSettings;
		WindSettings windSettings;
		GrabSettings grabSettings;
	};

	// ------------------------------------------------------------------------------------------------
	// Gravité Force 
	void calculForceGravite(Tissu::TissuSoA& tissu, const GravitySettings& gravitySettings)
	{
		#pragma omp parallel for
		for (int i = 0; i < static_cast<int>(tissu.accelerations.size()); i++) {
			if (tissu.positions[i].z >= tissu.tissuSettings.floorHeight
				&& tissu.isAffectedGravity[i]
				&& !tissu.isFixed[i])
			{
				tissu.accelerations[i] += gravitySettings.directionVector * gravitySettings.amplitude * (1.0f / tissu.tissuSettings.mass);
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	// Tension Force 
	glm::vec3 tensionF(
		const glm::vec3& point1,
		const glm::vec3& point2,
		const glm::vec3& vel1,
		const glm::vec3& vel2,
		const TensionSettings& tensionSettings,
		const float gap)
	{
		glm::vec3 direction = point2 - point1;

		float distance = glm::length(direction);

		// Éviter la division par zéro
		if (distance < 1e-6f) {
			return glm::vec3(0.0f);
		}

		// Force de ressort (Hooke)
		float displacement = distance - gap;
		glm::vec3 forceSpring = tensionSettings.force * displacement * direction;

		// Amortissement (damping)
		glm::vec3 relativeVelocity = vel2 - vel1;
		float dampingForce = glm::dot(relativeVelocity, direction);
		glm::vec3 forceDamping = tensionSettings.damping * dampingForce * direction;

		return forceSpring + forceDamping;
	}

	void calculForceTension(Tissu::TissuSoA& tissu, const TensionSettings& tensionSettings) {
		#pragma omp parallel for
		for (int i = 0; i < static_cast<int>(tissu.accelerations.size()); i++) {
			glm::vec3 forceTotale(0.0f);  //C'est une acceleration
			glm::vec3 p1 = tissu.positions[i];
			glm::vec3 vel1 = tissu.vitesses[i];
			for (size_t j = 0; j < tissu.linkedVertices[i].size(); j++) {
				glm::vec3 p2 = tissu.positions[std::get<size_t>(tissu.linkedVertices[i][j])];
				glm::vec3 vel2 = tissu.vitesses[std::get<size_t>(tissu.linkedVertices[i][j])];

				glm::vec3 force = tensionF(p1, p2, vel1, vel2, tensionSettings, std::get<float>(tissu.linkedVertices[i][j]));

				if (glm::length(force) > tensionSettings.maxTensionForce) {
					force = glm::normalize(force) * tensionSettings.maxTensionForce;
				}

				forceTotale += force;
			}
			if (!tissu.isFixed[i]) {
				tissu.accelerations[i] += forceTotale;// / tissu.masses[i];
			}
		}
	};

	// ------------------------------------------------------------------------------------------------
	// Wind Force 
	void calculForceWind(Tissu::TissuSoA& tissu, const WindSettings& windSettings) {
		if (!windSettings.isEnabled)
			return;
		glm::vec3 dir = glm::normalize(windSettings.directionVector);

		#pragma omp parallel for
		for (int i = 0; i < static_cast<int>(tissu.positions.size()); i++) {
			// Projection de la normale sur la direction du vent
			float dot = glm::dot(tissu.normals[i], dir);
			float surfaceFactor = glm::clamp(dot, 0.0f, 1.0f); // optionnel : unilatéral

			float variation = 1.0f;
			if (windSettings.isOscillate)
			{
				float omega = glm::two_pi<float>() * windSettings.oscillationFrequency; // 2πf
				variation = 0.5f * std::sin(omega * glfwGetTime()) + 0.5f;          // ∈ [0,1]
			}
			glm::vec3 force = dir * windSettings.amplitude * surfaceFactor * variation;
			tissu.accelerations[i] += force; // / tissu.masses[i];
		}

	}

	// ------------------------------------------------------------------------------------------------
	// Sphere Collision
	void applySphereCollisionResponse(Tissu::TissuSoA& tissu, const glm::vec3& sphereCenter, const float& sphereRadius, const CollisionSettings& collisionSettings) {
		#pragma omp parallel for
		for (int i = 0; i < static_cast<int>(tissu.positions.size()); i++) {
			glm::vec3 direction = tissu.positions[i] - sphereCenter;
			float distance = glm::length(direction);

			if (distance <= sphereRadius + collisionSettings.antiClippingGap) {
				float penetration = sphereRadius + collisionSettings.antiClippingGap - distance;
				glm::vec3 normal = (distance > 0.0f) ? glm::normalize(direction) : glm::vec3(0, 1, 0);
				glm::vec3 reaction_force = normal * penetration * collisionSettings.stiffness;
				tissu.accelerations[i] += reaction_force;
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
		Tissu::TissuSoA& tissu,
		const glm::vec3& boxCenter,
		const glm::vec3& boxSize,
		const glm::vec3& rotationAxis,
		const CollisionSettings& collisionSettings)
	{
		glm::mat3 rotationMatrix = glm::mat3(1.0f);
		if (glm::length(rotationAxis) > 1e-6f) {
			rotationMatrix = glm::mat3_cast(glm::angleAxis(glm::radians(0.0f), glm::normalize(rotationAxis)));
		}

		glm::mat3 invRotation = glm::transpose(rotationMatrix);
		glm::vec3 halfSize = (boxSize + glm::vec3(collisionSettings.antiClippingGap * 5.0f)) * 0.5f; // *0.5 car moitierbox // *10.0 car l'anticlipping doit etre plus grand pour une boite

		#pragma omp parallel for
		for (int i = 0; i < static_cast<int>(tissu.positions.size()); i++) {
			glm::vec3 localPos = invRotation * (tissu.positions[i] - boxCenter);
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
				tissu.accelerations[i] += worldNormal * (-dist) * collisionSettings.stiffness;
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	// Cylinder Collision (IA Genererated - Won't take credit for this)
	void applyCylinderCollisionResponse(
		Tissu::TissuSoA& tissu,
		const glm::vec3& cylinderCenter,
		const float& cylinderRadius,
		const float& cylinderHeight,
		const glm::vec3& axisDirection, // direction centrale du cylindre (doit être normalisée)
		const CollisionSettings& collisionSettings
	) {
		glm::vec3 axis = glm::normalize(axisDirection);
		float halfHeight = cylinderHeight * 0.5f;
		float radiusWithGap2 = glm::pow(cylinderRadius + collisionSettings.antiClippingGap, 2.0f);

		#pragma omp parallel for
		for (int i = 0; i < static_cast<int>(tissu.positions.size()); ++i) {
			glm::vec3 pos = tissu.positions[i];
			glm::vec3 posOld = tissu.positionsOld[i];
			glm::vec3 velocity = pos - posOld;

			// Vector from cylinder center to point
			glm::vec3 toPoint = pos - cylinderCenter;

			// Projection on cylinder axis
			float heightAlongAxis = glm::dot(toPoint, axis);

			// Check if inside height
			if (heightAlongAxis < -halfHeight || heightAlongAxis > halfHeight) continue;

			// Closest point on axis
			glm::vec3 axisPoint = cylinderCenter + axis * heightAlongAxis;
			glm::vec3 radialVec = pos - axisPoint;
			float radialDist2 = glm::dot(radialVec, radialVec);

			if (radialDist2 < radiusWithGap2) {
				float penetration = (cylinderRadius + collisionSettings.antiClippingGap) - glm::sqrt(radialDist2);
				glm::vec3 normal = (radialDist2 > 0.0f) ? glm::normalize(radialVec) : glm::vec3(1, 0, 0);

				// Réaction ressort
				glm::vec3 reaction_force = normal * penetration * collisionSettings.stiffness/1000;
				tissu.accelerations[i] += reaction_force;

				// Freinage normal (empêche rebond)
				glm::vec3 normal_velocity = glm::dot(velocity, normal) * normal;
				glm::vec3 damping_force = -collisionSettings.damping * normal_velocity;
				tissu.accelerations[i] += damping_force;
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	// Cone Collision (IA Genererated - Won't take credit for this)
	void applyConeCollisionResponse(
		Tissu::TissuSoA& tissu,
		const glm::vec3& coneTip,
		const glm::vec3& axisDirection,  // direction du cône (normalisée) : de la pointe vers la base
		float height,                    // hauteur totale du cône
		float baseRadius,                // rayon à la base
		const CollisionSettings& collisionSettings
	) {
		glm::vec3 axis = glm::normalize(axisDirection);
		float sinAngle = baseRadius / glm::sqrt(baseRadius * baseRadius + height * height); // sin(θ) du cône
		float cosAngle = glm::sqrt(1 - sinAngle * sinAngle);
		float tanAngle = baseRadius / height;
		float antiClip = collisionSettings.antiClippingGap;

		#pragma omp parallel for
		for (int i = 0; i < static_cast<int>(tissu.positions.size()); ++i) {
			glm::vec3& pos = tissu.positions[i];
			glm::vec3 velocity = tissu.positions[i] - tissu.positionsOld[i];

			glm::vec3 toPoint = pos - coneTip;
			float projLen = glm::dot(toPoint, axis);

			if (projLen < 0.0f || projLen > height) continue; // en dehors de la hauteur du cône

			glm::vec3 radial = toPoint - projLen * axis;
			float radialDist = glm::length(radial);
			float maxRadiusAtHeight = projLen * tanAngle + antiClip;

			if (radialDist < maxRadiusAtHeight) {
				// Il y a contact avec la paroi du cône
				float penetration = maxRadiusAtHeight - radialDist;
				glm::vec3 normal = (radialDist > 0.0f) ? glm::normalize(radial) : glm::vec3(1, 0, 0);

				// Force de réaction
				glm::vec3 reaction = normal * penetration * collisionSettings.stiffness;
				tissu.accelerations[i] += reaction;

				// Freinage (anti-rebond)
				glm::vec3 normalVel = glm::dot(velocity, normal) * normal;
				glm::vec3 damping = -collisionSettings.damping * normalVel;
				tissu.accelerations[i] += damping;
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	// Disk Collision
	void applyDiskCollisionResponse(
		Tissu::TissuSoA& tissu,
		const glm::vec3& diskCenter,
		const float& diskRadius,
		const glm::vec3& rotationAxis,
		const CollisionSettings& collisionSettings) {
		glm::vec3 diskNormal = glm::normalize(rotationAxis);
		float radiusWithGap2 = glm::pow(diskRadius + collisionSettings.antiClippingGap, 2.0f);

		#pragma omp parallel for
		for (int i = 0; i < static_cast<int>(tissu.positions.size()); i++) {
			glm::vec3 pos = tissu.positions[i];
			glm::vec3 posOld = tissu.positionsOld[i];
			glm::vec3 velocity = pos - posOld;
			glm::vec3 toPoint = pos - diskCenter;

			// Distance signée au plan du disque
			float distToPlane = glm::dot(toPoint, diskNormal);

			// Projection pour test radial
			glm::vec3 projected = pos - distToPlane * diskNormal;
			float radialDist2 = glm::length2(projected - diskCenter);

			if (distToPlane < 0.0f && radialDist2 <= radiusWithGap2) {
				float penetration = -distToPlane + collisionSettings.antiClippingGap;

				// Force de réaction (type ressort)
				glm::vec3 reaction_force = diskNormal * penetration * collisionSettings.stiffness/10000;
				tissu.accelerations[i] += reaction_force;

				// Force de freinage (amortissement vertical extrême)
				glm::vec3 vertical_velocity = glm::dot(velocity, diskNormal) * diskNormal;
				glm::vec3 damping_force = -collisionSettings.damping * vertical_velocity;
				tissu.accelerations[i] += damping_force;
			}
		}
	}
	

	// ------------------------------------------------------------------------------------------------
	// Grab Force
	void calculForceGrab(Tissu::TissuSoA& tissu, const GrabSettings& grabSettings)
	{
		if (Grabbing::isGrabbing && tissu.grabbedIndex != -1) {
			glm::vec3 targetPos = Grabbing::rayOrigin + Grabbing::rayDirection * tissu.grabDepth;
			glm::vec3 vel = tissu.positions[tissu.grabbedIndex] - tissu.positionsOld[tissu.grabbedIndex];

			glm::vec3 delta = targetPos - tissu.positions[tissu.grabbedIndex];

			glm::vec3 springForce = delta * grabSettings.force;
			glm::vec3 dampingForce = -grabSettings.damping * vel;

			glm::vec3 totalForce = springForce + dampingForce;

			tissu.accelerations[tissu.grabbedIndex] += totalForce;
		}
	}

	// ------------------------------------------------------------------------------------------------
	// Self Collision
	// Hash fonction pour glm::ivec3
	struct IVec3Hash {
		size_t operator()(const glm::ivec3& v) const {
			return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1) ^ (std::hash<int>()(v.z) << 2);
		}
	};

	void handleSelfCollisions(std::vector<glm::vec3>& positions,
		float minDistance,
		float cellSize) {

		std::unordered_map<glm::ivec3, std::vector<size_t>, IVec3Hash> spatialMap;
		float minDistance2 = minDistance * minDistance;

		// Étape 1 : assigner chaque point à une cellule

		{
			std::unordered_map<glm::ivec3, std::vector<size_t>, IVec3Hash> localMap;
			for (size_t i = 0; i < positions.size(); ++i) {
				glm::ivec3 cell = glm::floor(positions[i] / cellSize);
				#pragma omp critical
				spatialMap[cell].push_back(i);
			}
		}

		// Étape 2 : tester collisions entre cellules voisines
		for (int c = 0; c < static_cast<int>(spatialMap.size()); ++c) {
			auto it = std::next(spatialMap.begin(), c);
			const glm::ivec3& cell = it->first;
			const std::vector<size_t>& indices = it->second;

			for (int dx = -1; dx <= 1; ++dx) {
				for (int dy = -1; dy <= 1; ++dy) {
					for (int dz = -1; dz <= 1; ++dz) {
						glm::ivec3 neighborCell = cell + glm::ivec3(dx, dy, dz);
						auto neighborIt = spatialMap.find(neighborCell);
						if (neighborIt == spatialMap.end()) continue;

						const auto& neighbors = neighborIt->second;

						for (size_t i = 0; i < indices.size(); ++i) {
							size_t idxA = indices[i];
							for (size_t j = 0; j < neighbors.size(); ++j) {
								size_t idxB = neighbors[j];
								if (idxA >= idxB) continue; // éviter doublons

								glm::vec3 delta = positions[idxA] - positions[idxB];
								float dist2 = glm::dot(delta, delta);
								if (dist2 < minDistance2 && dist2 > glm::epsilon<float>()) {
									glm::vec3 dir = glm::normalize(delta);
									float penetration = minDistance - std::sqrt(dist2);
									glm::vec3 correction = 0.5f * penetration * dir;


									{
										positions[idxA] += correction;
										positions[idxB] -= correction;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// ------------------------------------------------------------------------------------------------
	// ------------------------------------------------------------------------------------------------
	// Main Calculation
	void applicationForces(Tissu::TissuSoA& LeTissu, const std::vector<MeshGenerator::MeshGenerator*>& listObj, const PhysicsSettings& physicsSettings)
	{
		// -------------------------------------------------------------------------------------
		// Acceleraction remise à zéro à chaque frame
		LeTissu.resetAcceleration();

		// -------------------------------------------------------------------------------------
		// Self Collision
		if (physicsSettings.selfCollisionSettings.isEnabled)
			handleSelfCollisions(LeTissu.positions, physicsSettings.selfCollisionSettings.minDistance, physicsSettings.selfCollisionSettings.cellSize);
		// -------------------------------------------------------------------------------------
		// Gravité
		if (physicsSettings.gravitySettings.isEnabled)
			calculForceGravite(LeTissu, physicsSettings.gravitySettings);
		// Tension
		if (physicsSettings.tensionSettings.isEnabled)
			calculForceTension(LeTissu, physicsSettings.tensionSettings);
		// Wind
		if (physicsSettings.windSettings.isEnabled)
			calculForceWind(LeTissu, physicsSettings.windSettings);
		// Collsion
		if (physicsSettings.collisionSettings.isEnabled) {
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
						applySphereCollisionResponse(LeTissu, sphere_obj->meshCenter, sphere_obj->radius, physicsSettings.collisionSettings);
					}
					break;
				}

				case MeshGenerator::PrimitiveType::BOX:
				{
					auto* box_obj = dynamic_cast<MeshGenerator::ProceduralBoxMesh*>(obj);
					if (box_obj)
					{
						applyBoxCollisionResponse_SDF(LeTissu, box_obj->meshCenter, box_obj->size, box_obj->rotationAxis, physicsSettings.collisionSettings);
					}
					break;
				}

				case MeshGenerator::PrimitiveType::DISK:
				{
					auto* disk_obj = dynamic_cast<MeshGenerator::ProceduralDiskMesh*>(obj);
					if (disk_obj)
					{
						applyDiskCollisionResponse(LeTissu, disk_obj->meshCenter, disk_obj->radius, disk_obj->rotationAxis, physicsSettings.collisionSettings);
					}
					break;
				}

				case MeshGenerator::PrimitiveType::CONE:
				{
					auto* cone_obj = dynamic_cast<MeshGenerator::ProceduralConeMesh*>(obj);
					if (cone_obj)
					{
						applyConeCollisionResponse(LeTissu, cone_obj->meshCenter, cone_obj->rotationAxis, cone_obj->height, cone_obj->radius, physicsSettings.collisionSettings);
					}
					break;
				}

				case MeshGenerator::PrimitiveType::CYLINDER:
				{
					auto* cylinder_obj = dynamic_cast<MeshGenerator::ProceduralCylinderMesh*>(obj);
					if (cylinder_obj)
					{
						applyCylinderCollisionResponse(LeTissu, cylinder_obj->meshCenter, cylinder_obj->radius, cylinder_obj->height, cylinder_obj->rotationAxis, physicsSettings.collisionSettings);
					}
					break;
				}
				}
			}
		}
		
		// Grab
		if (physicsSettings.grabSettings.isEnabled)
			calculForceGrab(LeTissu, physicsSettings.grabSettings);

		// -------------------------------------------------------------------------------------
		//Application de l'intégration de Verlet (+pression/+réaliste)
		//x(t + 1) = x(t) + [x(t) - x(t - 1)] + a(t) * dt²               [x(t) - x(t - 1)] c'est la vitesse
		#pragma omp parallel for
		for (int i = 0; i < LeTissu.accelerations.size(); i++) {
			if (!LeTissu.isFixed[i] && LeTissu.positions[i].z > LeTissu.tissuSettings.floorHeight)
			{
				LeTissu.positionsNew[i] = LeTissu.positions[i] + LeTissu.vitesses[i] + LeTissu.accelerations[i] * ((physicsSettings.temporalSettings.dt * physicsSettings.temporalSettings.dt) / physicsSettings.temporalSettings.substeps);
			}
			else if (!LeTissu.isFixed[i] && LeTissu.positions[i].z <= LeTissu.tissuSettings.floorHeight)
			{
				LeTissu.isFixed[i] = true;
				LeTissu.positionsNew[i] = LeTissu.positions[i]; // point fixe
				LeTissu.positionsNew[i].z = LeTissu.tissuSettings.floorHeight;
			}
			else
			{
				LeTissu.positionsNew[i] = LeTissu.positions[i]; // point fixe
			}
		}

		// -------------------------------------------------------------------------------------
		// Implementation Double Buffering pour limiter les déformations artificielles causées par le traitement séquentiel
		LeTissu.updateVertexPosition();  // (0.0f) -> NewPos -> Pos -> OldPos -> *Void*
		LeTissu.computeVitesses();
	}

	// Main Calculation Substep
	void physicCalculationSubsteped(Tissu::TissuSoA& tissu, const std::vector<MeshGenerator::MeshGenerator*>& listObj, const PhysicsSettings& physicsSettings) {
		for (int s = 0; s < physicsSettings.temporalSettings.substeps; ++s) {
			applicationForces(tissu, listObj, physicsSettings);
		}
	}
}
#endif
