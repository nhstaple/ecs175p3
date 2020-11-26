#version 330 core

// Interpolated values from the vertex shaders
in vec3 Normal_modelspace;

// Ouput data
out vec3 color;

void main(){

	color = Normal_modelspace * 0.5 + 0.5;

}
