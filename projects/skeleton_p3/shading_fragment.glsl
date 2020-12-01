#version 330 core

#define uwu true

// Interpolated values from the vertex shaders
in vec3 Normal_modelspace;
in vec3 Gouraud_color;
in vec3 pos;

// Ouput data
out vec3 color;

uniform mat4 MVP;
// add more uniforms for phong calculation
uniform vec3 cameraLocation; // f: eye location
uniform vec3 lightLocation; // x: location of light source
uniform float I_a; // ambient intensity
uniform float I_l; // light intensity
uniform vec3 k_a; // ambient color
uniform vec3 k_d; // diffuse color
uniform vec3 k_s; // specular color
uniform uint specularLevel;
uniform float K;

uniform bool enablePhong;
uniform bool enableGouraud;
uniform bool enableAmbient;
uniform bool enableDiffuse;
uniform bool enableSpecular;

uniform float time;

bool isZeroVector(vec3 v);
float computeFactor(vec3 I, vec3 v, float K);

vec3 Compute_Color(vec3 camera_pos, vec3 light_pos, vec3 position, vec3 normal);
vec3 Compute_I_ambient(vec3 k, float I);
vec3 Compute_I_diffuse(vec3 k, float I, float c_a, vec3 v, float K);
vec3 Compute_I_specular(vec3 k, float I, float c_b, uint n, vec3 v, float K);
vec3 Compute_Color(vec3 camera_pos, vec3 light_pos, vec3 position, vec3 normal);

void main() {
	// use gouraud
	if(!enablePhong && enableGouraud) {
		color = Gouraud_color;
	// compute phong
	} else if (enablePhong && !enableGouraud) {
		vec4 point = vec4(pos, 1);
		// vec4 point = vec4(gl_FragCoord);
		vec4 cam = vec4(cameraLocation.x, cameraLocation.y, cameraLocation.z, 1);
		vec4 light = vec4(lightLocation.x, lightLocation.y, lightLocation.z, 1);

		vec3 p = vec3(point.x, point.y, point.z);
		vec3 l = vec3(light.x, light.y, light.z);
		vec3 x = vec3(cam.x, cam.y, cam.z);

		color = Compute_Color(x, l, p, Normal_modelspace);
	} else {
		vec3 boop = Normal_modelspace;
		boop.x = Normal_modelspace.x * cos(time);
		boop.y = Normal_modelspace.y * sin(time);
		boop.z = Normal_modelspace.x * cos(time) + Normal_modelspace.y * sin(2 * time);
		color = clamp(boop * 0.99 + 0.01, 0, 1);
	}
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
	float c_b = dot(r, normalize(v));
	float coeff = K * length(x - p);
	if(uwu) {
		// K = 0;
		v = f - pos;
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
		I_diffuse = Compute_I_diffuse(material_color, I_l, c_a, v, coeff);
		if(!isZeroVector(I_diffuse)) n_ = n_ + 1;
	}

	vec3 I_specular = vec3(0, 0, 0);
	if(enableSpecular) {
		I_specular = Compute_I_specular(light_color, I_l, c_b, specularLevel, v, coeff);
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
