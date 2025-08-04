#version 330 core

// ------------------------------------------------------
// OUT
out vec4 FragColor;

// ------------------------------------------------------
// OUT 
in vec2 textureCoordinate; // each cooardinate of each vertex in the texture coordinate system
in vec4 vertexPos;
in vec3 vertexNormal;

// ------------------------------------------------------
// STRUCT
struct Material 
{
    //vec3 ambient; Redondant avec la texture diffuse
    sampler2D diffuse;
    sampler2D specular;
	sampler2D emissive;

    float shininess;
}; 

struct Light {
    vec3 position;
	vec3 direction; //Pas utiliser. Permet de faire des Spotlight directionnel & Des light à l'infini'
	float cutOff; //Pas utiliser.  C'est un angle ! 'Permet de faire des Spotlight ne fonctionnant que dans un cone | on conpare alors l'angle avec l'angle cree par la direction du spot et du veteur spot/vertexPos
	float outerCutOff; //Pas utiliser C'est aussi un angle. Il Permet de cree un second cone pour faire un smothing de l'intensité lumineuse entre les 2 cones'

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	float constant; //Pas utiliser. Litteralement P(x) = ax²+bx+c | Permet de faire une fonction d'atenuation de la lumiere en fct de la distance source/vertex avec 1/P(x)
	float linear;
	float quadratic;
};

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

// ------------------------------------------------------
// FONCTIONS PROTOTYPES
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

// ------------------------------------------------------
// UNIFORMS
uniform vec3 vertexBaseColor;
uniform Material material;
uniform Light light;
uniform vec3 viewPos;
uniform sampler2D ourTexture;

// ------------------------------------------------------
// MAIN

void main()
{
	// Ambient light
	vec3 ambientColor = light.ambient * vec3(texture(material.diffuse, textureCoordinate)).rgb;

	// Diffuse Light
	vec3 vertexPosition = vec3(vertexPos.x,vertexPos.y,vertexPos.z);
	vec3 lightDirection = normalize(light.position - vertexPosition);
	float diffuseAmplitude = max(dot(vertexNormal, lightDirection), 0.0);
	vec3 diffuseColor = diffuseAmplitude * light.diffuse * vec3(texture(material.diffuse, textureCoordinate)).rgb;

	// Specular Light
	vec3 viewDirection = normalize(viewPos - vertexPosition);
	vec3 reflectDirection = reflect(-lightDirection, vertexNormal);
	float specularAmplitude = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);
	vec3 specularColor = specularAmplitude * light.specular * vec3(texture(material.specular, textureCoordinate)).rgb;

	// Emissive light
	vec3 emissiveColor = vec3(texture(material.emissive, textureCoordinate)).rgb;

	// Total
	vec3 finalColor = vertexBaseColor * (ambientColor + diffuseColor + specularColor + emissiveColor);
	FragColor = vec4(finalColor, 1.0);  // * texture(ourTexture, textureCoordinate)
}

// ------------------------------------------------------
// FONCTIONS DEFINITIONS
// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, textureCoordinate));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, textureCoordinate));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, textureCoordinate));
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, textureCoordinate));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, textureCoordinate));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, textureCoordinate));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, textureCoordinate));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, textureCoordinate));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, textureCoordinate));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}


//Tuto
//https://learnopengl.com/code_viewer_gh.php?code=src/2.lighting/6.multiple_lights/6.multiple_lights.fs