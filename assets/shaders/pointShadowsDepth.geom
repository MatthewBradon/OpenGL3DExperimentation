#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 shadowMatrices[6];

out vec4 FragPos; // FragPos from Geometry shader (output per emitted vertex)

void main() {
    for (int face = 0; face < 6; face++) {
        gl_Layer = face; // Render to corresponding face of cubemap
        
        for (int i = 0; i < 3; i++) { // For each vertex of the triangle
            
            FragPos = gl_in[i].gl_Position; // Pass the original position to fragment shader
            
            gl_Position = shadowMatrices[face] * FragPos; // Transform to light's clip space
            
            EmitVertex();
        }
        
        EndPrimitive();
    }
}