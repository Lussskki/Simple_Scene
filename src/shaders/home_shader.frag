#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
in vec3 Normal;

uniform sampler2D texture_diffuse;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform float ambientStrength;

void main() {
    vec3 textureColor = texture(texture_diffuse, TexCoord).rgb;
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(-lightDirection);

    float diffuseAmount = max(dot(normal, lightDir), 0.0);
    vec3 ambient = ambientStrength * lightColor;
    vec3 diffuse = diffuseAmount * lightColor;

    FragColor = vec4(textureColor * (ambient + diffuse), 1.0);
}
