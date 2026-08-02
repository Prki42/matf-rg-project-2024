//#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec2 aTexCoords2;
layout (location = 4) in vec3 aTangent;
layout (location = 5) in vec3 aBitangent;

out vec2 TexCoords;
out vec2 TexCoords2;
out vec3 FragPos;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
    TexCoords = aTexCoords;
    TexCoords2 = aTexCoords2;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;
in vec2 TexCoords2;
in vec3 FragPos;
in mat3 TBN;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_emissive1;
uniform int texture_normal1_uv;
uniform int texture_specular1_uv;
uniform int texture_emissive1_uv;
uniform vec3 emissiveFactor;
uniform vec3 diffuseFactor;
uniform float shininess;
uniform float opacity;

#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 4

struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
    int castsShadows;
    float shadowFar;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    int castsShadows;
    mat4 lightSpaceMatrix;
};

uniform vec3 viewPos;
uniform int numPointLights;
uniform int numSpotLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform samplerCube pointShadowMaps[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform sampler2D spotShadowMaps[MAX_SPOT_LIGHTS];

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color, vec3 specColor, float shadow) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * color;

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * light.color * specColor;

    return (diffuse + specular) * (1.0 - shadow) * attenuation;
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color, vec3 specColor, float shadow) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * color;

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * light.color * specColor;

    return (diffuse + specular) * (1.0 - shadow) * attenuation * intensity;
}

void main() {
    // which coordinates to use for normals
    vec2 normalUV = texture_normal1_uv == 1 ? TexCoords2 : TexCoords;

    vec3 normal = texture(texture_normal1, normalUV).rgb * 2.0 - 1.0;
    normal = normalize(TBN * normal);

    // which coordinates to use for specular map
    vec2 specUV = texture_specular1_uv == 1 ? TexCoords2 : TexCoords;

    // which coordinates to use for emissive map
    vec2 emissiveUV = texture_emissive1_uv == 1 ? TexCoords2 : TexCoords;

    vec4 diffuseSample = texture(texture_diffuse1, TexCoords);
    vec3 color = diffuseSample.rgb * diffuseFactor;

    if (opacity < 1.0)
        discard;

    vec3 specColor = texture(texture_specular1, specUV).rgb;
    vec3 emission = texture(texture_emissive1, emissiveUV).rgb;
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = 0.1 * color;

    for (int i = 0; i < numPointLights; i++) {
        float shadow = 0.0;
        if (pointLights[i].castsShadows != 0) {
            vec3 fragToLight = FragPos - pointLights[i].position;
            vec3 lightDir = normalize(-fragToLight);
            float closestDepth = 1.0;
            if (i == 0) closestDepth = texture(pointShadowMaps[0], fragToLight).r;
            else if (i == 1) closestDepth = texture(pointShadowMaps[1], fragToLight).r;
            else if (i == 2) closestDepth = texture(pointShadowMaps[2], fragToLight).r;
            else if (i == 3) closestDepth = texture(pointShadowMaps[3], fragToLight).r;
            closestDepth *= pointLights[i].shadowFar;
            float currentDepth = length(fragToLight);
            float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
            shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
        result += calcPointLight(pointLights[i], normal, FragPos, viewDir, color, specColor, shadow);
    }

    for (int i = 0; i < numSpotLights; i++) {
        float spotShadow = 0.0;
        if (spotLights[i].castsShadows != 0) {
            vec4 fragPosLightSpace = spotLights[i].lightSpaceMatrix * vec4(FragPos, 1.0);
            vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
            projCoords = projCoords * 0.5 + 0.5;
            float closestDepth = 1.0;
            if (i == 0) closestDepth = texture(spotShadowMaps[0], projCoords.xy).r;
            else if (i == 1) closestDepth = texture(spotShadowMaps[1], projCoords.xy).r;
            else if (i == 2) closestDepth = texture(spotShadowMaps[2], projCoords.xy).r;
            else if (i == 3) closestDepth = texture(spotShadowMaps[3], projCoords.xy).r;
            float currentDepth = projCoords.z;
            vec3 spotLightDir = normalize(spotLights[i].position - FragPos);
            float bias = max(0.001 * (1.0 - dot(normal, spotLightDir)), 0.0001);
            spotShadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
            if (projCoords.z > 1.0) spotShadow = 0.0;
        }
        result += calcSpotLight(spotLights[i], normal, FragPos, viewDir, color, specColor, spotShadow);
    }

    result += emission * emissiveFactor;

    FragColor = vec4(result, 1);

    float emBright = dot(emission * emissiveFactor, vec3(0.2126, 0.7152, 0.0722));
    float totalBright = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if (emBright > 0.2 || totalBright > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
