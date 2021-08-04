#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

uniform float heightScale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
    // Number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));

    // Calculate the size of each layer
    float layerDepth = 1 / numLayers;

    // Depth of current layer
    float currentLayerDepth = 0.0;

    // The amount to shift the texture coordinates per layer
    vec2 P = viewDir.xy / viewDir.z * heightScale;
    vec2 deltaTexCoords = P / numLayers;

    // Get inital values
    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue) {
        // Shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;

        // Get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;

        // Get depth of next layer
        currentLayerDepth += layerDepth;
    }

    // Get texture coordinates before collision
    vec2 previousTexCoords = currentTexCoords + deltaTexCoords;

    // Get depth after and before collision
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, previousTexCoords).r - currentLayerDepth + layerDepth;

    // Interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = previousTexCoords * weight + currentTexCoords * (1 - weight);

    return finalTexCoords;
}

void main() {
    // Offset texture coordinates with parallax mapping
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = fs_in.TexCoords;

    texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0) {
        discard;
    }

    // Obtain normal from normal map in range [0, 1]
    vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
    // Transform normal vector to range [-1, 1]
    normal = normalize(normal * 2.0 - 1.0); // This normal is in tangent space

    // Diffuse color
    vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;

    // Ambient
    vec3 ambient = 0.1 * color;

    // Diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
