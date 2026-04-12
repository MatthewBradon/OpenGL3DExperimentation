#version 330 core

#define NR_POINT_LIGHTS 1
#define MAX_DIFFUSE 12
#define MAX_SPECULAR 3

out vec4 FragColor;

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

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform Material material;
uniform DirectionalLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform vec3 objectColor;
uniform vec3 cameraPosition;
uniform sampler2D shadowMap;
// uniform samplerCube skybox;

vec4 SampleDiffuse();
vec3 SampleSpecular();
vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 FragPosition, vec3 viewDir);
vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 FragPosition, vec3 viewDir);
vec3 calculateRefraction(vec3 viewDir, vec3 normal);
vec3 calculateReflection(vec3 viewDir, vec3 normal);
float LinearizeDepth(float depth);
float PhongSpecular(vec3 lightDir, vec3 normal, vec3 viewDir);
float BlinnPhongSpecular(vec3 lightDir, vec3 normal, vec3 viewDir);
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);
float LinearizeDepth(float depth);



float refractionRatio = 1.00 / 1.52;

void main() {
    
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(cameraPosition - fs_in.FragPos);

    vec4 diffuseColor = SampleDiffuse();

    vec3 result = calculateDirectionalLight(dirLight, norm, viewDir);

    // for(int i = 0; i < NR_POINT_LIGHTS; i++) {
    //     result += calculatePointLight(pointLights[i], norm, fs_in.FragPos, viewDir);
    // }


    result += calculateSpotLight(spotLight, norm, fs_in.FragPos, viewDir);    
    
    // skybox using specular highlights
    // vec3 viewDirection = normalize(fs_in.FragPos - cameraPosition);
    // result += calculateReflection(viewDirection, fs_in.Normal) * SampleSpecular();

    FragColor = vec4(result, diffuseColor.a);
}


vec4 SampleDiffuse() {
    vec4 color = vec4(0.0);
    for (int i = 0; i < material.diffuseCount; i++) {
    color += texture(material.texture_diffuse[i], fs_in.TexCoords);
    }
    // Average
    return color / max(material.diffuseCount, 1);
}

vec3 SampleSpecular() {
    vec3 color = vec3(0.0);
    for (int i = 0; i < material.specularCount; i++) {
    color += texture(material.texture_specular[i], fs_in.TexCoords).rgb;
    }
    return color / max(material.specularCount, 1);
}


vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
    // Directional light uses a constant direction for all fragments
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
	
    vec3 result = ambient + diffuse + specular;

    // Shadow calculation
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDir);

    return (ambient + (1.0 - shadow) * (diffuse + specular));
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

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Outside the light frustum/projection should be treated as lit
    if (projCoords.z > 1.0) return 0.0;
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) return 0.0;

    float closestDepth = texture(shadowMap, projCoords.xy).r; // Get depth from shadow map
    float currentDepth = projCoords.z;

    // Bias to prevent shadow acne
    float bias = max(0.0005 * (1.0 - dot(normal, lightDir)), 0.00005);

    float shadow = 0.0;
    // PCF (Percentage Closer Filtering) for softer shadows
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }
    }
    shadow /= 9.0;
    
    return shadow;
}

float LinearizeDepth(float depth) {
    float near = 0.1;
    float far = 100.0;
    return (2.0 * near) / (far + near - depth * (far - near));
}

// vec3 calculateRefraction(vec3 viewDir, vec3 normal ) {
//     vec3 refractionVector = refract(viewDir, normalize(normal), refractionRatio);
//     return texture(skybox, refractionVector).rgb;
// }

// vec3 calculateReflection(vec3 viewDir, vec3 normal) {
//     vec3 reflectionVector = reflect(viewDir, normalize(normal));
//     return texture(skybox, reflectionVector).rgb;
// }