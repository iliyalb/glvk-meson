#version 460

layout(location = 0) in vec3 inColor;
layout(location = 0) out vec4 fragColor;

void main() {
    vec3 normal = normalize(vec3(0.0, 0.0, 1.0));
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diffuse = max(dot(normal, lightDir), 0.0);
    diffuse = diffuse * 0.8 + 0.2;
    vec3 finalColor = inColor * diffuse;
    fragColor = vec4(finalColor, 1.0);
}
