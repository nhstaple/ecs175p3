#version 330 core

#define uwu false

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace; // p: position of reflection
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace; // n: normal vector

// Output data ; will be interpolated for each fragment.
out vec3 Normal_modelspace;
out vec3 Gouraud_color;
out vec3 pos;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

uniform vec3 cameraLocation; // f: eye location
uniform vec3 lightLocation; // x: location of light source
uniform float I_a; // ambient intensity
uniform float I_l; // light intensity
uniform vec3 k_a; // ambient color
uniform vec3 k_d; // diffuse color
uniform vec3 k_s; // specular color
uniform uint specularLevel;

uniform bool enableAmbient;
uniform bool enableDiffuse;
uniform bool enableSpecular;

bool isZeroVector(vec3 v);
float computeFactor(vec3 I, vec3 v, float K);
vec3 Compute_Color(vec3 camera_pos, vec3 light_pos, vec3 position, vec3 normal);
vec3 Compute_I_ambient(vec3 k, float I);
vec3 Compute_I_diffuse(vec3 k, float I, float c_a, vec3 v, float K);
vec3 Compute_I_specular(vec3 k, float I, float c_b, uint n, vec3 v, float K);

void main() {
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	pos = vec3(gl_Position.x, gl_Position.y, gl_Position.z);

	Normal_modelspace = normalize(vertexNormal_modelspace);
	vec4 cam = vec4(cameraLocation, 1);
	vec4 light = vec4(lightLocation, 1);
	light = light;

	vec3 p = vec3(gl_Position.x, gl_Position.y, gl_Position.z);
	vec3 l = vec3(light.x, light.y, light.z);
	vec3 x = vec3(cam.x, cam.y, cam.z);
	vec3 n = Normal_modelspace;

	Gouraud_color = Compute_Color(x, l, p, n);
}

bool isZeroVector(vec3 v) {
	return v.x == 0 && v.y == 0 && v.z == 0;
}

vec3 Compute_Color(vec3 camera_pos, vec3 light_pos, vec3 position, vec3 normal) {
	// phong model parameters
	vec3 p = position;
	vec3 n = normalize(Normal_modelspace);
	vec3 f = camera_pos;
	vec3 x = light_pos;
	vec3 l = normalize(x - p);
	vec3 v = normalize(f - p);
	vec3 r = normalize(-l + 2*(dot(n, l))*n);
	float c_a = dot(n, l);
	float c_b = dot(r, v);
	float K = length(x - p);

	if(uwu) {
		K = 0;
		v = f - p;
	}

	vec3 ambient_color = k_a;
	vec3 material_color = k_d;
	vec3 light_color = k_s;

	int n_ = 0;
	vec3 I_ambient = vec3(0, 0, 0);
	if(enableAmbient) {
		I_ambient = Compute_I_ambient(ambient_color, I_a);
		if(!isZeroVector(I_ambient)) n_ = n_ + 1;
	}
	vec3 I_diffuse = vec3(0, 0, 0);
	if(enableDiffuse) {
		I_diffuse = Compute_I_diffuse(material_color, I_l, c_a, v, K);
		if(!isZeroVector(I_diffuse)) n_ = n_ + 1;
	}

	vec3 I_specular = vec3(0, 0, 0);
	if(enableSpecular) {
		I_specular = Compute_I_specular(light_color, I_l, c_b, specularLevel, v, K);
		if(!isZeroVector(I_specular)) n_ = n_ + 1;
	}

	return clamp((I_ambient) + (I_diffuse) + (I_specular), 0, 1);
}

float computeFactor(float I, vec3 v, float K) {
	return I / (length(v) + K);
}

vec3 Compute_I_ambient(vec3 k, float I) {
	return k * I;
}

vec3 Compute_I_diffuse(vec3 k, float I, float c_a, vec3 v, float K) {
	if(c_a > 0 && I > 0) {
		return computeFactor(I, v, K) * k * c_a;
	} else {
		return vec3(0, 0, 0);
	}
}

vec3 Compute_I_specular(vec3 k, float I, float c_b, uint n, vec3 v, float K) {
	if(int(n) > 0 && I > 0 && c_b > 0) {
		return computeFactor(I, v, K) * k * pow(c_b, n);
	} else {
		return vec3(0, 0, 0);
	}
}
