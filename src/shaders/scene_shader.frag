#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform vec3 spotPosition;
uniform vec3 spotDirection;
uniform vec3 spotColor;
uniform float spotCutoff;
uniform float spotOuterCutoff;

void main() {
    vec3 textureColor = texture(texture_diffuse, TexCoord).rgb;
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(-lightDirection);

    float diffuseAmount = max(dot(normal, lightDir), 0.0);
    vec3 ambient = ambientStrength * lightColor;
    vec3 diffuse = diffuseAmount * lightColor;

    vec3 toSpotLight = normalize(spotPosition - FragPos);
    float theta = dot(toSpotLight, normalize(-spotDirection));
    float epsilon = spotCutoff - spotOuterCutoff;
    float spotIntensity = clamp((theta - spotOuterCutoff) / epsilon, 0.0, 1.0);

    float spotDiffuseAmount = max(abs(dot(normal, toSpotLight)), 0.0);
    vec3 spotlight = spotDiffuseAmount * spotColor * spotIntensity;

    FragColor = vec4(textureColor * (ambient + diffuse + spotlight), 1.0);
}
