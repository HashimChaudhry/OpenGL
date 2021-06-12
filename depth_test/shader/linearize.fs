#version 330 core
out vec4 FragColor;

float near = 0.1; 
float far = 100.0; 

float LinearizeDepth(float depth)  {
    float z = depth * 2.0 - 1.0; // back to NDC 

    // The following formula undoes what the projection matrix does, giving us a value
    // from [near, far], where near and far are the values of the near and far plane distances
    // in the perspective view; it then normalizes them to be in range [0,1]
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main() {             
    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far to get depth in range [0,1] for visualization purposes
    FragColor = vec4(vec3(depth), 1.0);
}
