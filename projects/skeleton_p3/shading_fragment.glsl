#version 330 core

// Interpolated values from the vertex shaders
in vec3 Normal_modelspace;

// Ouput data
out vec3 color;

// add more uniforms for phong calculation
uniform vec3 cameraLocation; // f: eye location
uniform vec3 lightLocation; // x: location of light source
uniform vec3 I_a; // ambient intensity
uniform float k_a; // ambient coefficient
uniform float k_d; // diffuse coefficient
uniform vec3 lightColor; // the intensity of the light
uniform float k_s; // specular coefficient
uniform int specularLevel;

uniform bool enablePhong;
uniform bool enableAmbient;
uniform bool enableDiffuse;
uniform bool enableSpecular;

vec3 computeFactor(vec3 I, vec3 v, float K);
bool isZeroVector(vec3 v);

vec3 Compute_I_ambient(float k, vec3 I);
vec3 Compute_I_diffuse(vec3 I, vec3 v, float K, float k, float c_a);
vec3 Compute_I_specular(vec3 I, vec3 v, float K, float k, float c_b, int n);

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
		float c_b = dot(r, v);
		float K = length(x - p);

		vec3 ambient_color = I_a;
		vec3 light_color = lightColor;

		/*
		// calculate I_p
		// I_ambient
		vec3 I_ambient = vec3(0, 0, 0);
		if(enableAmbient) {
			I_ambient = ambient_color * k_a;
		}

		vec3 I_diffuse = vec3(0, 0, 0);
		// I_diffuse
		if(enableDiffuse && c_a != 0) {
			I_diffuse = (k_d * c_a) * (computeFactor(light_color, v, length(x - p)));
			// I_diffuse = (k_d * c_a) * (computeFactor(light_color, v, 0));
		}

		vec3 I_specular = vec3(0, 0, 0);
		// I_specular
		if(enableSpecular && c_b != 0 && specularLevel > 0) {
			I_specular = k_s * pow(c_b, specularLevel) * (computeFactor(light_color, v, length(x - p)));
			// I_specular = k_s * pow(c_b, specularLevel) * (computeFactor(light_color, v, 0));
		}

		vec3 I_p;
		bool inc_ambient = enableAmbient && (I_ambient.x != 0 || I_ambient.y != 0 || I_ambient.z != 0);
		bool inc_diffuse = enableDiffuse && (I_diffuse.x != 0 || I_diffuse.y != 0 || I_diffuse.z != 0);
		bool inc_specular = enableSpecular && (I_specular.x != 0 || I_specular.y != 0 || I_specular.z != 0);
		if(false) {
			vec3 x1 = inc_ambient ? normalize(I_ambient) : vec3(0, 0, 0);
			vec3 x2 = inc_diffuse ? normalize(I_diffuse) : vec3(0, 0, 0);
			vec3 x3 = inc_specular ? normalize(I_specular) : vec3(0, 0, 0);

		if(inc_ambient && inc_diffuse && inc_specular) {
			I_p = normalize(I_ambient + I_diffuse + I_specular);
		} else if(inc_ambient && inc_diffuse && !inc_specular) {
			I_p = normalize(I_ambient + I_diffuse);
		} else if(inc_ambient && !inc_diffuse && inc_specular) {
			I_p = normalize(I_ambient + I_specular);
		} else if(inc_ambient && !inc_diffuse && !inc_specular)  {
			I_p = normalize(I_ambient);
		} else if(!inc_ambient && inc_diffuse && inc_specular) {
			I_p = normalize(I_diffuse + I_specular);
		} else if(!inc_ambient && !inc_diffuse && inc_specular) {
			I_p = normalize(I_specular);
		} else if(!inc_ambient && inc_diffuse && !inc_specular) {
			I_p = normalize(I_diffuse);
		} else {
			// I_p = Normal_modelspace * 0.5 + 0.5;
		}

			I_p = normalize(x1 + x2 + x3);
		} else {
			vec3 x1 = inc_ambient ? (I_ambient) : vec3(0, 0, 0);
			vec3 x2 = inc_diffuse ? (I_diffuse) : vec3(0, 0, 0);
			vec3 x3 = inc_specular ? (I_specular) : vec3(0, 0, 0);
			I_p = normalize(x1 + x2 + x3);
		}
		color = I_p;
	} else {
		color = Normal_modelspace * 0.5 + 0.5;
	}
	*/

		vec3 I_ambient = enableAmbient ? Compute_I_ambient(k_a, ambient_color) : vec3(0, 0, 0);
		vec3 I_diffuse = c_a != 0 ? Compute_I_diffuse(light_color, v, K, k_d, c_a) : vec3(0, 0, 0);
		vec3 I_specular = c_b != 0 ? Compute_I_specular(light_color, v, K, k_s, c_b, specularLevel) : vec3(0, 0, 0);

		vec3 I_p = I_ambient;
		I_p = enableDiffuse ? I_p + I_diffuse : I_p;
		I_p = enableSpecular ? I_p + I_specular : I_p;
		color = I_p;
	} else {
		color = Normal_modelspace * 0.5 + 0.5;
	}
}
vec3 computeFactor(vec3 I, vec3 v, float K) {
	return I / (length(v) + K);
}

bool isZeroVector(vec3 v) {
	return v.x == 0 && v.y == 0 && v.z == 0;
}

vec3 Compute_I_ambient(float k, vec3 I) {
	return k * I;
}

vec3 Compute_I_diffuse(vec3 I, vec3 v, float K, float k, float c_a) {
	return computeFactor(I, v, K) * k * c_a;
}

vec3 Compute_I_specular(vec3 I, vec3 v, float K, float k, float c_b, int n) {
	return computeFactor(I, v, K) * k * pow(c_b, n);
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