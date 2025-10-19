#version 330 core
layout(location = 0) in vec3 aPos;
out vec3 vertexColor;
uniform int rotation; // 0, 1, 2 to indicate how many steps to rotate

void main() {
    gl_Position = vec4(aPos, 1.0);

    // Original colors for vertices 0, 1, 2
    vec3 colors[3];
    colors[0] = vec3(1.0, 0.0, 0.0); // Red
    colors[1] = vec3(0.0, 1.0, 0.0); // Green
    colors[2] = vec3(0.0, 0.0, 1.0); // Blue

    // Use gl_VertexID to index vertices, rotate using uniform
    int idx = (gl_VertexID + rotation) % 3;
    vertexColor = colors[idx];
}
