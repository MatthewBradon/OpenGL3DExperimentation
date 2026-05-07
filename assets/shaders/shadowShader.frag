#version 330 core

#define NR_POINT_LIGHTS 1
#define MAX_DIFFUSE 12
#define MAX_SPECULAR 3
#define MAX_NORMAL 1
#define MAX_HEIGHT 1
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse[MAX_DIFFUSE];
    sampler2D texture_specular[MAX_SPECULAR];
    sampler2D texture_normal[MAX_NORMAL];
    sampler2D texture_height[MAX_HEIGHT];
    int diffuseCount;
    int specularCount;
    int normalCount;
    int heightCount;
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
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    mat3 TBN;
    vec3 TangentFragPos;
    vec3 TangentCameraPos;
} fs_in;

uniform Material material;
uniform DirectionalLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform vec3 objectColor;
uniform vec3 cameraPosition;
uniform sampler2D shadowMap;
uniform samplerCube skybox;
// Single global cube shadow map for point lights
uniform samplerCube shadowCubeMap;
uniform bool showPointShadowMap;
uniform bool useParallaxMapping;

vec2 adjustedTexCoords;

vec4 SampleDiffuse();
vec3 SampleSpecular();
vec3 SampleNormal();
vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 FragPosition, vec3 viewDir);
vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 FragPosition, vec3 viewDir);
vec3 calculateRefraction(vec3 viewDir, vec3 normal);
vec3 calculateReflection(vec3 viewDir, vec3 normal);
float PhongSpecular(vec3 lightDir, vec3 normal, vec3 viewDir);
float BlinnPhongSpecular(vec3 lightDir, vec3 normal, vec3 viewDir);
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);
float ShadowCalculationPointLight(vec3 fragPos, PointLight light);
float LinearizeDepth(float depth);
vec2 ParallaxOcclusionMapping(vec2 texCoords, vec3 viewDir);

vec3 sampleOffsetDirections[20] = vec3[]
(
    vec3( 1, 1, 1), vec3( 1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
    vec3( 1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3( 1, 1, 0), vec3( 1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3( 1, 0, 1), vec3(-1, 0, 1), vec3( 1, 0, -1), vec3(-1, 0, -1),
    vec3( 0, 1, 1), vec3( 0, -1, 1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

vec3 SampleNormal() {
    return texture(material.texture_normal[0], adjustedTexCoords).rgb;
}

float refractionRatio = 1.00 / 1.52;

void main() {

    bool useNormalMap = material.normalCount > 0;

    // start with geometric normal in world space
    vec3 norm = normalize(fs_in.Normal);

    // if there's a normal map, sample tangent-space normal and convert to world space
    if (useNormalMap) {
        vec3 nmap = SampleNormal();
        nmap = normalize(nmap * 2.0 - 1.0);
        // fs_in.TBN maps tangent -> world, so multiply to get world-space normal
        norm = normalize(fs_in.TBN * nmap);

        // If normal mapping is enabled, also apply parallax occlusion mapping to adjust texture coordinates
        // Use adjustedTexCoords (do NOT write to fs_in)
        adjustedTexCoords = fs_in.TexCoords;
        if (useParallaxMapping) {
            vec3 T_viewDir = normalize(fs_in.TangentCameraPos - fs_in.TangentFragPos);
            vec2 parallaxTexCoords = ParallaxOcclusionMapping(adjustedTexCoords, T_viewDir);
            adjustedTexCoords = parallaxTexCoords;
        }

    } else {
        adjustedTexCoords = fs_in.TexCoords;
    }

    vec3 viewDir = normalize(cameraPosition - fs_in.FragPos);
    vec4 diffuseColor = SampleDiffuse();

    // call lighting functions with world-space normals/view direction so shadows remain world-space
    // vec3 result = calculateDirectionalLight(dirLight, norm, viewDir);
    vec3 result = vec3(0.0);

    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += calculatePointLight(pointLights[i], norm, fs_in.FragPos, viewDir);
    }

    result += calculateSpotLight(spotLight, norm, fs_in.FragPos, viewDir);    

    if (!showPointShadowMap) {
        FragColor = vec4(result, diffuseColor.a);
    }
}


vec4 SampleDiffuse() {
    vec4 color = vec4(0.0);
    for (int i = 0; i < material.diffuseCount; i++) {
    color += texture(material.texture_diffuse[i], adjustedTexCoords);
    }
    // Average
    return color / max(material.diffuseCount, 1);
}

vec3 SampleSpecular() {
    vec3 color = vec3(0.0);
    for (int i = 0; i < material.specularCount; i++) {
    color += texture(material.texture_specular[i], adjustedTexCoords).rgb;
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

    float shadow = ShadowCalculationPointLight(FragPosition, light);

    
    return ambient + (1.0 - shadow) * (diffuse + specular);
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


float ShadowCalculationPointLight(vec3 fragPos, PointLight light) {
    vec3 fragToLight = fragPos - light.position;
    float currentDepth = length(fragToLight);
    float viewDistance = length(cameraPosition - fragPos);
    float bias = 0.1; // Bias to prevent shadow acne
    float shadow = 0.0;

    

    float diskRadius = (1.0 + (viewDistance / light.far_plane)) / 25.0;

    for (int i = 0; i < sampleOffsetDirections.length(); ++i) {
        float closestDepth = texture(shadowCubeMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= light.far_plane; // Convert back to world units

        shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
    }

    if (showPointShadowMap) {
        float depth = texture(shadowCubeMap, fragToLight).r;
        depth *= light.far_plane; // Convert back to world units
        FragColor = vec4(vec3(depth / light.far_plane), 1.0); // Visualize depth in shadow map
    }

    return shadow / float(sampleOffsetDirections.length());
}

vec2 ParallaxOcclusionMapping(vec2 texCoords, vec3 viewDir)
{
	const float minLayers = 8.0;
	const float maxLayers = 32.0;
	
	float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));

	// calculate the size of each layer
	float layerDepth = 1.0 / numLayers;

	// depth of current layer
	float currentLayerDepth = 0.0;

    // Use the material height map sampler directly (samplers must be uniform globals)
    float height_scale = 20.0;

	// amount to shift the texture coordinates per layer (from vector P)
	vec2 P = viewDir.xy * height_scale;
	vec2 deltaTexCoords = P / numLayers;
	
	// get initial values
	vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(material.texture_height[0], currentTexCoords).r;
	
	while(currentLayerDepth < currentDepthMapValue)
	{
		// shift texture coordinates along direction of P
		currentTexCoords -= deltaTexCoords;
		
		// get depthmap value at current texture coordinates
        currentDepthMapValue = texture(material.texture_height[0], currentTexCoords).r;
		
		// get depth of next layer
		currentLayerDepth += layerDepth;
	}
	
	// get texture coordinates before collision (reverse operations)
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
	
	// get depth after and before collision for linear interpolation
	float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(material.texture_height[0], prevTexCoords).r - currentLayerDepth + layerDepth;
	
	// interpolation of texture coordinates
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
	return finalTexCoords;
}


// vec3 calculateRefraction(vec3 viewDir, vec3 normal ) {
//     vec3 refractionVector = refract(viewDir, normalize(normal), refractionRatio);
//     return texture(skybox, refractionVector).rgb;
// }

// vec3 calculateReflection(vec3 viewDir, vec3 normal) {
//     vec3 reflectionVector = reflect(viewDir, normalize(normal));
//     return texture(skybox, reflectionVector).rgb;
// }