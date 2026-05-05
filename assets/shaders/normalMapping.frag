#version 330 core

#define NR_POINT_LIGHTS 1
#define MAX_DIFFUSE 12
#define MAX_SPECULAR 3

out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse[MAX_DIFFUSE];
    sampler2D texture_specular[MAX_SPECULAR];
    sampler2D texture_normal;
    int diffuseCount;
    int specularCount;
    int normalCount;
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
    float far_plane;
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
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentCameraPos;
    vec3 TangentFragPos;
} fs_in;

uniform Material material;
uniform DirectionalLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
// Single global cube shadow map for point lights
uniform bool showPointShadowMap;

vec4 SampleDiffuse();
vec3 SampleSpecular();
vec3 SampleNormal();

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 FragPosition, vec3 viewDir);
vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 FragPosition, vec3 viewDir);

float PhongSpecular(vec3 lightDir, vec3 normal, vec3 viewDir);
float BlinnPhongSpecular(vec3 lightDir, vec3 normal, vec3 viewDir);


vec3 sampleOffsetDirections[20] = vec3[]
(
    vec3( 1, 1, 1), vec3( 1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
    vec3( 1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3( 1, 1, 0), vec3( 1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3( 1, 0, 1), vec3(-1, 0, 1), vec3( 1, 0, -1), vec3(-1, 0, -1),
    vec3( 0, 1, 1), vec3( 0, -1, 1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);


void main() {
    
    vec3 norm = SampleNormal();
    vec3 viewDir = normalize(fs_in.TangentCameraPos - fs_in.TangentFragPos);

    vec4 diffuseColor = SampleDiffuse();

    vec3 result = calculateDirectionalLight(dirLight, norm, viewDir);

    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += calculatePointLight(pointLights[i], norm, fs_in.TangentFragPos, viewDir);
    }

    result += calculateSpotLight(spotLight, norm, fs_in.TangentFragPos, viewDir);    

    if (!showPointShadowMap) {
        FragColor = vec4(result, diffuseColor.a);
    }
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

vec3 SampleNormal() {
    vec3 normalMap = texture(material.texture_normal, fs_in.TexCoords).rgb;
    return normalize(normalMap * 2.0 - 1.0);
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
