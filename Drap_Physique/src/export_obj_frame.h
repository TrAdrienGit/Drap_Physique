#ifndef EXPORT_OBJ_FRAME_H
#define EXPORT_OBJ_FRAME_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <glm/glm.hpp>

void exportFrameAsOBJ(const std::vector<glm::vec3>& positions,
    const std::vector<unsigned int>& indices,
    const std::string& outputDir,
    int frameNumber) {
    // Format du nom de fichier : frame_XXXX.obj
    std::ostringstream filename;
    filename << outputDir << "/frame_" << std::setw(4) << std::setfill('0') << frameNumber << ".obj";

    std::ofstream out(filename.str());
    if (!out.is_open()) {
        std::cerr << "Erreur : impossible d'ouvrir le fichier d'export : " << filename.str() << "\n";
        return;
    }

    // Exporter les sommets
    for (const auto& pos : positions) {
        out << "v " << pos.x << " " << pos.y << " " << pos.z << "\n";
    }

    // Exporter les faces (triangles)
    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        // Les indices OBJ commencent à 1
        out << "f " << (indices[i] + 1) << " " << (indices[i + 1] + 1) << " " << (indices[i + 2] + 1) << "\n";
    }

    out.close();
    //std::cout << "Frame " << frameNumber << " exportée : " << filename.str() << "\n";
}


#endif