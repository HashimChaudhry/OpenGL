#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform bool hdr;
uniform float exposure;

void main() {
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;

    if(hdr) {
        // Reinhard
        // vec3 result = hdrColor (hdrColor + vec3(1.0))

        // Exposure
        vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

        // Also gamma correct
        result = pow(result, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    } else {
        vec3 result = pow(hdrColor, vec3(1.0/gamma));
        FragColor = vec4(result, 1.0);
    }
}