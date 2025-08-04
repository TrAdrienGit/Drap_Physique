#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
layout (location = 1) in vec3 normalVec;
layout (location = 2) in vec2 textureVec;
  
out vec2 textureCoordinate;
out vec4 vertexPos;   // IN WORLD-COORD
out vec3 vertexNormal;




uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;



void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos, 1.0);
	textureCoordinate = textureVec;
	vec4 vertexPos = modelMatrix * vec4(aPos, 1.0);
	vertexNormal = normalize(normalVec);
}