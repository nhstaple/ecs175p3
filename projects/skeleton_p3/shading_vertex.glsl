#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace; // p: position of reflection
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace; // n: normal vector

// Output data ; will be interpolated for each fragment.
out vec3 Normal_modelspace;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main() {
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	Normal_modelspace = normalize(vertexNormal_modelspace);
}
