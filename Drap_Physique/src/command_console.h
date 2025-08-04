#ifndef COMMAND_CONSOLE_H
#define COMMAND_CONSOLE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <atomic>
#include <iostream>
#include <thread>
#include <iostream>
#include <sstream>
#include <filesystem>

#include "tissuSoA.h"
#include "physicSoA.h"
#include "glencapsulation.h"

// ----------------------------------------------------------------------------------------------------

std::atomic<bool> running(true);

std::string recordsPath = "recording";

bool clearFolder(const std::string& folderPath) {
    try {
        if (!std::filesystem::exists(folderPath)) {
            std::cerr << "Folder does not exist: " << folderPath << "\n";
            return false;
        }

        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            std::filesystem::remove_all(entry.path());  // Supprime fichier ou dossier récursivement
        }

        return true;
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";
        return false;
    }
}

struct SimulationState {
    Tissu::TissuSoA& tissu;

    std::atomic<bool> pauseSimulation = false;
    std::atomic<bool> pauseCamera = false;

    std::atomic<bool> gravityEnabled = true;
    std::atomic<bool> tensionEnabled = true;
    std::atomic<bool> collisionEnabled = true;
    std::atomic<bool> selfCollisionEnabled = false;
    std::atomic<bool> windEnabled = true;
    std::atomic<bool> grabEnabled = true;

    std::atomic<bool> tissuEnabled = true;
    std::atomic<bool> sphereEnabled = true;
    std::atomic<bool> boxEnabled = true;
    std::atomic<bool> cylinderEnabled = true;
    std::atomic<bool> coneEnabled = true;
    std::atomic<bool> diskEnabled = true;

    std::atomic<bool> tissuWireframeEnabled = false;
    std::atomic<bool> sphereWireframeEnabled = false;
    std::atomic<bool> boxWireframeEnabled = false;
    std::atomic<bool> cylinderWireframeEnabled = false;
    std::atomic<bool> coneWireframeEnabled = true;
    std::atomic<bool> diskWireframeEnabled = false;

    //glm::vec3 tissuMove = glm::vec3(0.0f);
    //glm::vec3 sphereMove = glm::vec3(0.0f);
    //glm::vec3 boxMove = glm::vec3(0.0f);
    //glm::vec3 cylinderMove = glm::vec3(0.0f);
    //glm::vec3 coneMove = glm::vec3(0.0f);

    //glm::vec3 tissuSetPos = glm::vec3(0.0f);
    //glm::vec3 sphereSetPos = glm::vec3(0.0f);
    //glm::vec3 boxSetPos = glm::vec3(0.0f);
    //glm::vec3 cylinderSetPos = glm::vec3(0.0f);
    //glm::vec3 coneSetPos = glm::vec3(0.0f);

    std::atomic<bool> recordingEnabled = false;
};

auto setBoolFlag = [&](std::atomic<bool>& flag, bool value, const std::string& name) {
    flag = value;
    std::cout << name << " set to " << (value ? "enabled" : "disabled") << "\n";
    };

void commandThread(SimulationState* sim)
{
    std::string input;
    while (running) {
        std::getline(std::cin, input);

        std::istringstream iss(input);
        std::string command, target;
        iss >> command >> target;

        // -----------------

        if (command == "resetPos") {
            sim->tissu.resetPosition();
            std::cout << "   Tissu reseted" << std::endl;
        }
        else if (command == "lockCorner") {
            sim->tissu.lockCorner(true);
            std::cout << "   Corner locked" << std::endl;
        }
        else if (command == "unlockCorner") {
            sim->tissu.lockCorner(false);
            std::cout << "   Corner unlocked" << std::endl;
        }
        else if (command == "lockSide") {
            sim->tissu.lockSide(true);
            std::cout << "   Corner locked" << std::endl;
        }
        else if (command == "unlockSide") {
            sim->tissu.lockSide(false);
            std::cout << "   Corner unlocked" << std::endl;
        }
        else if (command == "fixVertex") {
            float i, j;
            if (!(iss >> i >> j)) {
                std::cout << "Usage: " << command << " fixVertex <i> <j>\n";
            }
            else {
                sim->tissu.isFixed[sim->tissu.getIndex(i,j)] = true;
            }
        }
        else if (command == "unlockVertex") {
            float i, j;
            if (!(iss >> i >> j)) {
                std::cout << "Usage: " << command << " unfixVertex <i> <j>\n";
            }
            else {
                sim->tissu.isFixed[sim->tissu.getIndex(i, j)] = false;
            }
        }
        else if (command == "pause") {
            sim->pauseSimulation = !sim->pauseSimulation;
        }
        else if (command == "pauseCamera") {
            sim->pauseCamera = !sim->pauseCamera;
        }
        else if (command == "startRecording") {
            sim->recordingEnabled = true;
            std::cout << "   recording Enabled" << std::endl;
        }
        else if (command == "stopRecording") {
            sim->recordingEnabled = false;
            std::cout << "   recording Stopped" << std::endl;
        }

        // -----------------

        else if (command == "enable" || command == "disable") {
            bool value = (command == "enable");

            if (target == "gravity")             setBoolFlag(sim->gravityEnabled, value, "gravity");
            else if (target == "tension")        setBoolFlag(sim->tensionEnabled, value, "tension");
            else if (target == "collision")      setBoolFlag(sim->collisionEnabled, value, "collision");
            else if (target == "selfCollision")  setBoolFlag(sim->selfCollisionEnabled, value, "selfCollision");
            else if (target == "wind")           setBoolFlag(sim->windEnabled, value, "wind");
            else if (target == "grab")           setBoolFlag(sim->grabEnabled, value, "grab");

            else if (target == "tissu")          setBoolFlag(sim->tissuEnabled, value, "tissu");
            else if (target == "sphere")         setBoolFlag(sim->sphereEnabled, value, "sphere");
            else if (target == "box")            setBoolFlag(sim->boxEnabled, value, "box");
            else if (target == "cylinder")       setBoolFlag(sim->cylinderEnabled, value, "cylinder");
            else if (target == "cone")           setBoolFlag(sim->coneEnabled, value, "cone");
            else if (target == "disk")           setBoolFlag(sim->coneEnabled, value, "disk");

            else if (target == "tissuWire")      setBoolFlag(sim->tissuWireframeEnabled, value, "tissuWire");
            else if (target == "sphereWire")     setBoolFlag(sim->sphereWireframeEnabled, value, "sphereWire");
            else if (target == "boxWire")        setBoolFlag(sim->boxWireframeEnabled, value, "boxWire");
            else if (target == "cylinderWire")   setBoolFlag(sim->cylinderWireframeEnabled, value, "cylinderWire");
            else if (target == "coneWire")       setBoolFlag(sim->coneWireframeEnabled, value, "coneWire");
            else if (target == "diskWire")       setBoolFlag(sim->coneWireframeEnabled, value, "diskWire");

            else std::cout << "   Unknown flag: " << target << "\n";
        }

        // -----------------

        //else if (command == "move" || command == "setpos") {
        //    float x, y, z;
        //    if (!(iss >> x >> y >> z)) {
        //        std::cout << "Usage: " << command << " <target> <x> <y> <z>\n";
        //    }
        //    else {
        //        glm::vec3 val(x, y, z);

        //        if (target == "tissu")
        //            (command == "move") ? sim->tissuMove = val : sim->tissuSetPos = val;
        //        else if (target == "sphere")
        //            (command == "move") ? sim->sphereMove = val : sim->sphereSetPos = val;
        //        else if (target == "box")
        //            (command == "move") ? sim->boxMove = val : sim->boxSetPos = val;
        //        else if (target == "cylinder")
        //            (command == "move") ? sim->cylinderMove = val : sim->cylinderSetPos = val;
        //        else if (target == "cone")
        //            (command == "move") ? sim->coneMove = val : sim->coneSetPos = val;
        //        else
        //            std::cout << "Unknown object: " << target << "\n";
        //    }
        //}

        // -----------------

        else if (command == "deleteRecords") {
            clearFolder(recordsPath);
            std::cout << "   Records succesfully deleted" << std::endl;
        }

        // -----------------

        else if (command == "help") {
            std::cout << "Available commands:\n";
            std::cout << "  resetPos                    -> Resets the cloth position\n";
            std::cout << "  lockCorner / unlockCorner   -> Locks or unlocks the corners of the cloth\n";
            std::cout << "  lockSide / unlockSide       -> Locks or unlocks the borders of the cloth\n";
            std::cout << "  fixVertex <i> <j>           -> Fixes vertex at position (i, j)\n";
            std::cout << "  unlockVertex <i> <j>        -> Unlocks vertex at position (i, j)\n";
            std::cout << "  pause                       -> Toggles simulation pause\n";
            std::cout << "  pauseCamera                 -> Toggles camera movement pause\n";
            std::cout << "  startRecording              -> Starts recording the simulation\n";
            std::cout << "  stopRecording               -> Stops the recording\n";
            std::cout << "  enable <option>             -> Enables a simulation feature (see below)\n";
            std::cout << "  disable <option>            -> Disables a simulation feature\n";
            std::cout << "    Options: gravity, tension, collision, selfCollision, wind, grab\n";
            std::cout << "             tissu, sphere, box, cylinder, cone, disk\n";
            std::cout << "             tissuWire, sphereWire, boxWire, cylinderWire, coneWire, diskWire\n";
            std::cout << "  deleteRecords               -> Erase all files in the recording folder\n";
            std::cout << "  exit                        -> Exits the simulation\n";
            std::cout << "  help                        -> Displays this help message\n";
            }

        // -----------------

        else if (command == "exit") {
            running = false;
        }
        else {
            std::cout << "[Unknown command]: " << command << "\n";
        }
    }
}




void transfereData(SimulationState& sim, Physics::PhysicsSettings& physicsSettings) {
    physicsSettings.gravitySettings.isEnabled = sim.gravityEnabled;
    physicsSettings.tensionSettings.isEnabled = sim.tensionEnabled;
    physicsSettings.collisionSettings.isEnabled = sim.collisionEnabled;
    physicsSettings.selfCollisionSettings.isEnabled = sim.selfCollisionEnabled;
    physicsSettings.windSettings.isEnabled = sim.windEnabled;
    physicsSettings.grabSettings.isEnabled = sim.grabEnabled;
}












#endif