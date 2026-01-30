#version 330 core

#define NR_POINT_LIGHTS 1
#define MAX_DIFFUSE 8
#define MAX_SPECULAR 8

out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

struct Material {
    sampler2D texture_diffuse[MAX_DIFFUSE];
    sampler2D texture_specular[MAX_SPECULAR];
    int diffuseCount;
    int specularCount;
    float shininess;
};

struct DirectionalLight {
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



uniform Material material;
uniform DirectionalLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform vec3 objectColor;
uniform vec3 viewPos;

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float LinearizeDepth(float depth);

void main() {
    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 result = calculateDirectionalLight(dirLight, norm, viewDir);

    // for(int i = 0; i < NR_POINT_LIGHTS; i++) {
    //     result += calculatePointLight(pointLights[i], norm, FragPos, viewDir);
    // }


    result += calculateSpotLight(spotLight, norm, FragPos, viewDir);    
    
    FragColor = vec4(result, 1.0);
}


vec3 SampleDiffuse() {
    vec3 color = vec3(0.0);
    for (int i = 0; i < material.diffuseCount; i++) {
        color += texture(material.texture_diffuse[i], TexCoord).rgb;
    }
    // Average
    return color / max(material.diffuseCount, 1);
}

vec3 SampleSpecular() {
    vec3 color = vec3(0.0);
    for (int i = 0; i < material.specularCount; i++) {
        color += texture(material.texture_specular[i], TexCoord).rgb;
    }
    return color / max(material.specularCount, 1);
}


vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
    // Make the light direction from the fragment towards the light
    vec3 lightDir = normalize(-light.direction);
	
    // Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	
    // Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	
    vec3 diffuseTex  = SampleDiffuse();
    vec3 specularTex = SampleSpecular();


    // Apply
	vec3 ambient = light.ambient * diffuseTex;
	vec3 diffuse = light.diffuse * diff * diffuseTex;
	vec3 specular = light.specular * spec * specularTex;
	
    return ambient + diffuse + specular;
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    
    vec3 lightDir = normalize(light.position - fragPos);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // Attenuation
    float lightDistance = length(light.position - fragPos);
    float attenuation = 1.0 / (
        light.constant +
        light.linear * lightDistance +
        light.quadratic * (lightDistance * lightDistance)
    );


    vec3 diffuseTex  = SampleDiffuse();
    vec3 specularTex = SampleSpecular();

    // Apply
    vec3 ambient = light.ambient * diffuseTex;
    vec3 diffuse = light.diffuse * diff * diffuseTex;
    vec3 specular = light.specular * spec * specularTex;
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return ambient + diffuse + specular;
}


vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    
    vec3 lightDir = normalize(light.position - fragPos);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // Attenuation
    float lightDistance = length(light.position - fragPos);
    float attenuation = 1.0 / (
        light.constant +
        light.linear * lightDistance +
        light.quadratic * (lightDistance * lightDistance)
    );


    // Spotlight Intensity
    float theta = dot(lightDir, normalize(-light.direction)); // Cosine value between the direction of light (SpotDir) and the direction from the fragment position to the light
    float epsilon = light.cutOff - light.outerCutOff; // cosine values
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);


    vec3 diffuseTex  = SampleDiffuse();
    vec3 specularTex = SampleSpecular();


    // Apply
    vec3 ambient = light.ambient * diffuseTex;
    vec3 diffuse = light.diffuse * diff * diffuseTex;
    vec3 specular = light.specular * spec * specularTex;
    
    ambient *= attenuation;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return ambient + diffuse + specular;
}