#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include "shader.h"

namespace Light
{
    struct LightSettings
    {
        glm::vec3 ambient = glm::vec3(1.0f);
        glm::vec3 diffuse = glm::vec3(0.5f);
        glm::vec3 specular = glm::vec3(1.0f);
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    };


    void sendToShader(LightSettings& lightSettings, Shader& shader)
    {
            shader.sendUniformVec3("light.ambient", lightSettings.ambient);
            shader.sendUniformVec3("light.diffuse", lightSettings.diffuse);
            shader.sendUniformVec3("light.specular", lightSettings.specular);
            shader.sendUniformVec3("light.position", lightSettings.position);
    }

}
#endif