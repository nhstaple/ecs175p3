#version 330 core

// Interpolated values from the vertex shaders
in vec3 Normal_modelspace;

// Ouput data
out vec3 color;

// add more uniforms for phong calculation
uniform vec3 cameraLocation; // f: eye location
uniform vec3 lightLocation; // x: location of light source
uniform float I_a; // ambient intensity
uniform float I_l; // light intensity
uniform vec3 k_a; // ambient color
uniform vec3 k_d; // diffuse color
uniform vec3 k_s; // specular color
uniform uint specularLevel;

uniform bool enablePhong;
uniform bool enableAmbient;
uniform bool enableDiffuse;
uniform bool enableSpecular;

bool isZeroVector(vec3 v);
float computeFactor(vec3 I, vec3 v, float K);

vec3 Compute_I_ambient(vec3 k, float I);
vec3 Compute_I_diffuse(vec3 k, float I, float c_a, vec3 v, float K);
vec3 Compute_I_specular(vec3 k, float I, float c_b, uint n, vec3 v, float K);

void main() {
	if (enablePhong) {
		// phong model parameters
		vec3 p = vec3(gl_FragCoord.x, gl_FragCoord.y, gl_FragCoord.z);
		vec3 n = normalize(Normal_modelspace);
		vec3 f = cameraLocation;
		vec3 x = lightLocation;
		vec3 l = normalize(x - p);
		vec3 v = normalize(f - p);
		vec3 r = -l + 2*(dot(n, l))*n;
		float c_a = dot(n, l);
		float c_b = dot(normalize(r), v);
		float K = length(x - p);
		if(true) { K = 0; }

		vec3 ambient_color = k_a;
		vec3 material_color = k_d;
		vec3 light_color = k_s;

		vec3 I_ambient = vec3(0, 0, 0);
		if(enableAmbient) I_ambient = Compute_I_ambient(ambient_color, I_a);

		vec3 I_diffuse = vec3(0, 0, 0);
		if(enableDiffuse) I_diffuse = Compute_I_diffuse(material_color, I_l, c_a, v, K);

		vec3 I_specular = vec3(0, 0, 0);
		if(enableSpecular) I_specular = Compute_I_specular(light_color, I_l, c_b, specularLevel, v, K);

		vec3 I_p = (I_ambient + I_diffuse + I_specular);

		color = I_p;
	} else {
		color = Normal_modelspace * 0.5 + 0.5;
	}
}

bool isZeroVector(vec3 v) {
	return v.x == 0 && v.y == 0 && v.z == 0;
}

float computeFactor(float I, vec3 v, float K) {
	return I / (length(v) + K);
}

vec3 Compute_I_ambient(vec3 k, float I) {
	return k * I;
}

vec3 Compute_I_diffuse(vec3 k, float I, float c_a, vec3 v, float K) {
	return computeFactor(I, v, K) * k * c_a;
}

vec3 Compute_I_specular(vec3 k, float I, float c_b, uint n, vec3 v, float K) {
	if(int(n) > 0 && c_b > 0) {
		return computeFactor(I, v, K) * k * pow(c_b, n);
	} else {
		return vec3(0, 0, 0);
	}
}

/*
void main(){
	// enableSpecular = true;
	if(enablePhong && (enableAmbient || enableDiffuse || enableSpecular)) {
		// the resulting stuffs
		vec3 I_p = vec3(0, 0, 0);

		// ambient
		if (enableAmbient) {
			I_p += k_a * I_a;
		}
		if(enableDiffuse || enableSpecular) {
			// get other variables used for phong
			vec3 p = vec3(gl_FragCoord.x, gl_FragCoord.y, gl_FragCoord.z);
			vec3 n = normalize(Normal_modelspace);
			vec3 f = cameraLocation;
			vec3 x = lightLocation;
			vec3 l = normalize(x - p);
			vec3 v = normalize(f - p);
			vec3 r = -l + 2*(dot(n, l))*n;

			// diffuse & specular
			bool cutDiffuse = dot(n, l) == 0;
			bool cutSpecular = dot(r, v) == 0;
			if (!cutDiffuse|| !cutSpecular) {
				// vec3 I_l = lightColor;
				vec3 factor = normalize(lightColor) / (length(f - p) + length(x - p));

				// diffuse
				if (enableDiffuse && !cutDiffuse) {
					I_p += factor * (k_d * dot(n, l));
				}
				// specular
				if (enableSpecular && !cutSpecular) {
					I_p += factor * (k_s * pow(dot(r, v), specularLevel));
				}
			}
		}
		if(I_p.x != 0 || I_p.y != 0 || I_p.z != 0) {
			color = I_p;
		} else {
			color = Normal_modelspace * 0.5 + 0.5;
		}
	} else {
		color = Normal_modelspace * 0.5 + 0.5;
	}
}
*/