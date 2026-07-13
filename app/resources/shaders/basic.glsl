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
uniform float shininess;

#define MAX_POINT_LIGHTS 8
#define MAX_SPOT_LIGHTS 4

struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
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
};

uniform vec3 viewPos;
uniform int numPointLights;
uniform int numSpotLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color, vec3 specColor) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * color;

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * light.color * specColor;

    return (diffuse + specular) * attenuation;
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color, vec3 specColor) {
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

    return (diffuse + specular) * attenuation * intensity;
}

void main() {
    // which coordinates to use for normals
    vec2 normalUV = texture_normal1_uv == 1 ? TexCoords2 : TexCoords;

    vec3 normal = texture(texture_normal1, normalUV).rgb * 2.0 - 1.0;
    normal = normalize(TBN * normal);

    // which coordinates to use for specular map
    vec2 specUV = texture_specular1_uv == 1 ? TexCoords2 : TexCoords;
    vec2 emissiveUV = texture_emissive1_uv == 1 ? TexCoords2 : TexCoords;

    // currently diffuse always uses TexCoords - should be changed later for generality
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    vec3 specColor = texture(texture_specular1, specUV).rgb;
    vec3 emission = texture(texture_emissive1, emissiveUV).rgb;
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = 0.07 * color;

    for (int i = 0; i < numPointLights; i++) {
        result += calcPointLight(pointLights[i], normal, FragPos, viewDir, color, specColor);
    }

    for (int i = 0; i < numSpotLights; i++) {
        result += calcSpotLight(spotLights[i], normal, FragPos, viewDir, color, specColor);
    }

    result += emission * emissiveFactor;

    // useful for debugging
    // FragColor = vec4(normal * 0.5 + 0.5, 1.0);
    // FragColor = vec4(texture(texture_normal1, normalUV).rgb, 1.0);
    // FragColor = vec4(TBN[0] * 0.5 + 0.5, 1.0);
    // FragColor = vec4(TBN[2] * 0.5 + 0.5, 1.0);

    FragColor = vec4(result, 1.0);

    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
