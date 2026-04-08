#version 330 core

#define NR_POINT_LIGHTS 1
#define MAX_DIFFUSE 8
#define MAX_SPECULAR 8

out vec4 FragColor;
in vec3 FragPosition;
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
uniform vec3 cameraPosition;
uniform samplerCube skybox;

vec4 SampleDiffuse();
vec3 SampleSpecular();
vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 FragPosition, vec3 viewDir);
vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 FragPosition, vec3 viewDir);
float LinearizeDepth(float depth);
float PhongSpecular(vec3 lightDir, vec3 normal, vec3 viewDir);
float BlinnPhongSpecular(vec3 lightDir, vec3 normal, vec3 viewDir);

float refractionRatio = 1.00 / 1.52;

void main() {
    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(cameraPosition - FragPosition);

    vec4 diffuseColor = SampleDiffuse();

    vec3 result = calculateDirectionalLight(dirLight, norm, viewDir);

    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += calculatePointLight(pointLights[i], norm, FragPosition, viewDir);
    }


    result += calculateSpotLight(spotLight, norm, FragPosition, viewDir);    
    
    // Reflection of skybox
    // vec3 viewDirection = normalize(FragPosition - cameraPosition);
    // vec3 reflectionVector = reflect(viewDirection, normalize(Normal));

    // result += texture(skybox, reflectionVector).rgb;

    // skybox using specular highlights
    vec3 viewDirection = normalize(FragPosition - cameraPosition);
    vec3 reflectionVector = reflect(viewDirection, normalize(Normal));
    result += texture(skybox, reflectionVector).rgb * SampleSpecular();

    // Refraction of skybox
    // vec3 refractionVector = refract(viewDirection, normalize(Normal), refractionRatio);
    // result += texture(skybox, refractionVector).rgb;

    FragColor = vec4(result, diffuseColor.a);
}


vec4 SampleDiffuse() {
    vec4 color = vec4(0.0);
    for (int i = 0; i < material.diffuseCount; i++) {
        color += texture(material.texture_diffuse[i], TexCoord);
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
    float spec = BlinnPhongSpecular(lightDir, normal, viewDir);
	
    vec3 diffuseTex  = SampleDiffuse().rgb;
    vec3 specularTex = SampleSpecular();


    // Apply
	vec3 ambient = light.ambient * diffuseTex;
	vec3 diffuse = light.diffuse * diff * diffuseTex;
	vec3 specular = light.specular * spec * specularTex;
	
    return ambient + diffuse + specular;
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 FragPosition, vec3 viewDir) {
    
    vec3 lightDir = normalize(light.position - FragPosition);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular
    float spec = BlinnPhongSpecular(lightDir, normal, viewDir);

    // Attenuation
    float lightDistance = length(light.position - FragPosition);
    float attenuation = 1.0 / (
        light.constant +
        light.linear * lightDistance +
        light.quadratic * (lightDistance * lightDistance)
    );


    vec3 diffuseTex  = SampleDiffuse().rgb;
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


vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 FragPosition, vec3 viewDir) {
    
    vec3 lightDir = normalize(light.position - FragPosition);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular
    float spec = BlinnPhongSpecular(lightDir, normal, viewDir);

    // Attenuation
    float lightDistance = length(light.position - FragPosition);
    float attenuation = 1.0 / (
        light.constant +
        light.linear * lightDistance +
        light.quadratic * (lightDistance * lightDistance)
    );


    // Spotlight Intensity
    float theta = dot(lightDir, normalize(-light.direction)); // Cosine value between the direction of light (SpotDir) and the direction from the fragment position to the light
    float epsilon = light.cutOff - light.outerCutOff; // cosine values
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);


    vec3 diffuseTex  = SampleDiffuse().rgb;
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

float PhongSpecular(vec3 lightDir, vec3 normal, vec3 viewDir) {
    vec3 reflectDir = reflect(-lightDir, normal);
    return pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
}

float BlinnPhongSpecular(vec3 lightDir, vec3 normal, vec3 viewDir) {
    vec3 halfDir = normalize(lightDir + viewDir);
    return pow(max(dot(normal, halfDir), 0.0), material.shininess);
}