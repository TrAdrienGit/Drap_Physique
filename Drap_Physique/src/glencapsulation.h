#ifndef GLENCAPSULATION_H
#define GLENCAPSULATION_H

#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include "shader.h"

// ----------------------------------------------------------------------------------------------------

namespace GL
{
	// ------------------------------------------------------------------------------------------------
	// Enumération pour les types de texture
	enum class TextureFormat {
		JPEG,
		JPG,
		PNG
	};

	// ------------------------------------------------------------------------------------------------
	class Texture
	{
	public:
		// Attributes
		GLuint id;
		const std::string path;
		TextureFormat format;

		// Constructor
		Texture(const std::string& texture_path, TextureFormat format) :
			path(texture_path), format(format) {
			loadTexture();
		}

		// Destructor
		~Texture() {
			glDeleteTextures(1, &this->id);
		}

	private:
		// Utility Methods
		void loadTexture() 
		{
			glGenTextures(1, &this->id);
			glBindTexture(GL_TEXTURE_2D, this->id);

			// Configuration de base
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			int width, height, nrChannels;
			unsigned char* data = stbi_load(this->path.c_str(), &width, &height, &nrChannels, 0);
			if (data) {
				GLenum format = (this->format == TextureFormat::PNG && nrChannels == 4) ? GL_RGBA : GL_RGB;
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else {
				std::cerr << "Erreur: impossible de charger la texture: " << this->path << std::endl;
			}
			stbi_image_free(data);
		}

	};

	// ------------------------------------------------------------------------------------------------
	class Material
	{
	public:
		Material(const Material&) = delete; //Empêche la copie d’un Material via le constructeur de copie (Material b = a; ou Material b(a);).
		Material& operator=(const Material&) = delete; //Empêche l’affectation par copie (b = a;).
		Material(Material&&) = default; //Autorise le déplacement (move constructor) via std::move.
		Material& operator=(Material&&) = default; //Permet aussi l’affectation par déplacement :

		// Attributes
		std::shared_ptr<Texture> diffuse_texture;
		std::shared_ptr<Texture> specular_texture;
		std::shared_ptr<Texture> emissive_texture;
		float shininess;
		glm::vec3 baseColor;

		// Constructor
		Material(std::shared_ptr<Texture> diffuse_texture, std::shared_ptr<Texture> specular_texture, std::shared_ptr<Texture> emissive_texture, float shiny = 34.0f, glm::vec3 baseColor = glm::vec3(1.0f)) :
			diffuse_texture(diffuse_texture), specular_texture(specular_texture), emissive_texture(emissive_texture), shininess(shiny), baseColor(baseColor) {}

		// Destructor
		~Material() {
		}
	};

	// ------------------------------------------------------------------------------------------------
	class Mesh {
	public:
		// Attribute
		std::vector<GLfloat> VBOBuffer;
		std::vector<unsigned int> EBOBuffer;

		unsigned int VAO = 0;
		unsigned int VBO = 0;
		unsigned int EBO = 0;

		// Constructor
		Mesh(const std::vector<GLfloat>& vertices, const std::vector<unsigned int>& indices)
			: VBOBuffer(vertices), EBOBuffer(indices) {
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);
		}

		// Desructor
		~Mesh() {
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &EBO);
			glDeleteVertexArrays(1, &VAO);
		}

		// Utility Methods
		void sendVBOBuffer()
		{
			static constexpr int floatsPerVertex = 8;
			int stride = floatsPerVertex * sizeof(float);
			// Open VAO
			glBindVertexArray(this->VAO);
			// Update
			glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
			glBufferData(GL_ARRAY_BUFFER, this->VBOBuffer.size() * sizeof(GLfloat), this->VBOBuffer.data(), GL_DYNAMIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

			// Unbind Buffer
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			// Close VAO
			glBindVertexArray(0);
		}
		void sendEBOBuffer()
		{
			// Open VAO
			glBindVertexArray(this->VAO);
			// Update EBO
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->EBOBuffer.size() * sizeof(int), this->EBOBuffer.data(), GL_DYNAMIC_DRAW);
			// Unbind Buffer
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			// Close VAO
			glBindVertexArray(0);
		}
		void draw() {
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(EBOBuffer.size()), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	};

	// ------------------------------------------------------------------------------------------------
	class Model
	{
	public:
		// Attributes
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
		std::shared_ptr<Mesh> mesh;

		// Constructor
		Model(std::shared_ptr<Mesh> mesh,
			glm::vec3 position = glm::vec3(0.0f),
			glm::vec3 rotation = glm::vec3(0.0f),
			glm::vec3 scale = glm::vec3(1.0f))
			: mesh(mesh), position(position), rotation(rotation), scale(scale) {
		}

		// Utility Methods
		glm::mat4 getModelMatrix() const
		{
			glm::mat4 model(1.0f);
			model = glm::translate(model, position);
			model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
			model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
			model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
			model = glm::scale(model, scale);
			return model;
		}
	};

	// ------------------------------------------------------------------------------------------------
	// ------------------------------------------------------------------------------------------------
	class Renderable
	{
	public:
		// Attribute
		Model model; // -> EBO,VBO buffer
		Material material;
		Shader shader;

		glm::mat4 modelMatrix = glm::mat4(1.0f);
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;

		bool wireframeDisplay;

		// Constructeur
		Renderable(Model&& model, Material&& material, Shader& shader, glm::vec2 screenSize = glm::vec2(1280, 720), bool wireframeDisplay = false)
			: model(std::move(model)), material(std::move(material)), shader(std::move(shader)), wireframeDisplay(wireframeDisplay)
		{
			sendMaterial();
			projectionMatrix = glm::perspective(glm::radians(45.0f), (float)screenSize.x / (float)screenSize.y, 0.1f, 1000.0f);
		}

		// Utility Methods
		const void sendMaterial()
		{
			// Active le shader
			this->shader.use();
			// Lie les textures aux unités de texture
			glActiveTexture(GL_TEXTURE0); // Active l'unité de texture 1
			glBindTexture(GL_TEXTURE_2D, this->material.diffuse_texture->id); // Lie la deuxième texture
			glActiveTexture(GL_TEXTURE1); // Active l'unité de texture 2
			glBindTexture(GL_TEXTURE_2D, this->material.specular_texture->id); // Lie la 3eme texture
			glActiveTexture(GL_TEXTURE2); // Active l'unité de texture 3
			glBindTexture(GL_TEXTURE_2D, this->material.emissive_texture->id); // Lie la 4eme texture
			// Indique au shader quelles unités de texture utiliser
			this->shader.sendUniformInt("material.diffuse", 0); // texture2 correspond à l'unité de texture 1
			this->shader.sendUniformInt("material.specular", 1); // texture3 correspond à l'unité de texture 2
			this->shader.sendUniformInt("material.emissive", 2); // texture3 correspond à l'unité de texture 3
			this->shader.sendUniformFloat("material.shininess", material.shininess);
			this->shader.sendUniformVec3("vertexBaseColor", material.baseColor);
		}

		const void sendMatrix()
		{
			shader.sendUniformMatrix4("modelMatrix", modelMatrix);
			shader.sendUniformMatrix4("viewMatrix", viewMatrix);
			shader.sendUniformMatrix4("projectionMatrix", projectionMatrix);
		}

		void draw()
		{
			shader.use();
			sendMaterial();
			sendMatrix();

			(this->wireframeDisplay) ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			this->model.mesh->draw();
		}
	};

	// ------------------------------------------------------------------------------------------------
}













#endif