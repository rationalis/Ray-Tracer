#include <chrono>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <stack>
#include "Transform.h"
#include <FreeImage.h>
#include <tuple>
#include <utility>

#include "glm/ext.hpp"

using namespace std;
using glm::max;
using glm::min;
using glm::radians;

// Main variables in the program.  
#define MAINPROGRAM 
#include "variables.h" 
#include "readfile.h" // prototypes for readfile.cpp

//static bool debug = false;
static float epsilon = 0.0000001f;

ray rayThruPixel(int i, int j) {
	float width = (float)w;
	float height = (float)h;
	float y = i + 0.5f;
	float x = j + 0.5f;
	float tanfovy2 = sin(radians(fovy) / 2) / cos(radians(fovy) / 2);
	float tanfovx2 = tanfovy2 * width / height;
	float alpha = tanfovx2 * (x - (width / 2)) / (width / 2);
	float beta = tanfovy2 * ((height / 2) - y) / (height / 2);

	return ray{ vec4(0,0,0,1), vec4(alpha, beta, -1, 0) };
}

float raySphereIntersect(const ray& ray, const vec3& center, float r) {
	float t = -1;
	vec3 p0 = dehomo(ray.origin);
	vec3 p1 = dehomo(ray.direction);

	float a = dot(p1, p1);
	float b = 2 * dot(p1, p0 - center);
	float c = dot(p0 - center, p0 - center) - (r * r);
	float d = (b*b) - (4 * a*c);

	if (d < 0) {
		return t;
	}
	float t1 = (-b + sqrt(d)) / (2 * a);
	float t2 = (-b - sqrt(d)) / (2 * a);

	if (t1 > epsilon && t2 > epsilon) { t = t1 < t2 ? t1 : t2; }
	else if (t1 > epsilon) { t = t1; }
	else if (t2 > epsilon) { t = t2; }
	/**
	if (p0 != vec3(0, 0, 0) && debug) {
		std::cout <<
			"ray origin: " << to_string(p0) << "\n" <<
			"t1: " << t1 << "\n" <<
			"t2: " << t2 << "\n";
	}**/
	return t;
}

float rayTriangleIntersect(const ray& ray, const triangle& tri) {
	const vec3& v1 = tri.v1_t;
	const vec3& n = tri.n_t;

	vec3 p0 = dehomo(ray.origin);
	vec3 p1 = dehomo(ray.direction);
	float t = -1;
	float d = dot(p1, n);
	if (d == 0) {
		return t;
	}
	t = dot(v1 - p0, n) / d;
	vec3 p = p0 + t*p1;
	if (p.x < tri.min_x || p.x > tri.max_x ||
		p.y < tri.min_y || p.y > tri.max_y ||
		p.z < tri.min_z || p.z > tri.max_z)
		return -1;
	// solve for beta, gamma using Cramer's Rule
	vec3 u2 = p - v1;
	float d1 = tri.d1;
	float d2 = tri.d2;
	float d3 = tri.d3;
	float d4 = dot(u2, tri.u0);
	float d5 = dot(u2, tri.u1);
	float denom = d1 * d3 - d2 * d2;
	float beta = (d3 * d4 - d2 * d5) / denom;
	float gamma = (d1 * d5 - d2 * d4) / denom;
	float alpha = 1.0f - beta - gamma;

	return (beta >= 0 && beta <= 1
		&& gamma >= 0 && gamma <= 1
		&& beta + gamma <= 1)
		? t : -1;
}

pair<float, object> intersect(const ray& r) {
	bool unset = true;
	float min_t = -1;
	object min_t_obj;

	//bool not_camera_ray = r.origin != vec4(0, 0, 0, 1);

	for (const auto& obj : objs) {
		float t = -1;
		switch (obj.tag) {
		case object::SPHERE: {
			const sphere& s = obj.sphere;
			const mat4& m = s.mvt;
			const mat4& m2 = s.mvti;
			ray r2 { m2 * r.origin, m2 * r.direction };
			float t0 = raySphereIntersect(r2, s.center, s.radius);
			if (t0 > 0) {
				vec3 v = dehomo(m * (r2.origin + t0 * r2.direction));
				t = (v - dehomo(r.origin))[0] / r.direction[0];
				//if (debug) std::cout << "t0: " << t0 << "\n";
				//if (debug) std::cout << "t calculated from t0: " << t << "\n";
			}
		} break;
		case object::TRIANGLE: {
			t = rayTriangleIntersect(r, obj.triangle);
		} break;
		default: {
			std::cout << "Intersecting against invalid object type...\n";
			getchar();
		}
		//case object::NORMAL_TRIANGLE: break;
		}
		if (t > epsilon && (unset || t < min_t)) {
			min_t = t;
			min_t_obj = obj;
			unset = false;
			//if (debug && not_camera_ray) std::cout << "min_t set to: " << min_t << "\n";
		}
	}

	/*if (debug && not_camera_ray) {
		std::cout << "unset: " << unset << "\n";
		std::cout << "returning min_t: " << min_t << "\n";
	}*/

	return make_pair(min_t, min_t_obj);
}

inline vec3 shadingEquation(const vec3& direction, const vec3& light, const vec3& normal, const vec3& halfvec, const thing& t) {
	float nDotL = dot(normal, direction);
	vec3 lambert = t.diffuse * light * max(nDotL, 0.0f);
	float nDotH = dot(normal, halfvec);
	vec3 phong = t.specular * light * pow(max(nDotH, 0.0f), t.shininess);
	return lambert + phong;
}

vec3 findColor(const ray& r, float t, const object& o, int depth) {
	if (t < 0) return vec3(0, 0, 0);

	vec3 ray_origin = dehomo(r.origin);
	vec3 ray_direction = dehomo(r.direction);

	thing thi;
	vec3 normal;
	const vec3 mypos = ray_origin + t * ray_direction;
	const vec4 mypos4(mypos, 1);

	switch (o.tag) {
	case object::SPHERE: {
		sphere s = o.sphere;
		normal = dehomo(s.mvti * mypos4) - s.center;
		thi = s;
		normal = normalize(thi.mvtit * normal);
	} break;
	case object::TRIANGLE: {
		triangle tri = o.triangle;
		normal = tri.n_t;
		thi = tri;
	} break;
	}

	vec3 color = depth > 1 ? vec3(0,0,0) : thi.ambient + thi.emission;
	const vec3 eyedirn = normalize(-ray_direction);

	for (const auto& light : lights) {
		bool is_point_light = light.type == light::POINT;
		vec3 direction = light.direction;
		if (is_point_light) direction -= mypos;

		float t_light = length(direction);

		vec3 normed_direction = normalize(direction);
		
		vec4 shadow_direction(normed_direction, 0);
		ray shadow{ mypos4 + 0.001f * shadow_direction, shadow_direction };

		//if (debug) std::cout << "Checking for shadow intersection\n";
		float t_blocked = intersect(shadow).first;
		//if (debug) std::cout << "Done checking for shadow intersection\n";
		if (t_blocked > epsilon && (!is_point_light || t_blocked < t_light)) continue;

		vec3 total_dirn = normed_direction + eyedirn;
		vec3 update = 
			shadingEquation(normed_direction,
				light.rgb,
				normal,
				normalize(total_dirn),
				thi);
		if (is_point_light) {
			update /= (light.const_attenuation
				+ light.linear_attenuation * t_light
				+ light.quadratic_attenuation * t_light * t_light);
		}
		/**
		if (debug) {
			std::cout << "light type: " << (light.type == light::POINT ? "POINT" : "DIRECTIONAL") << "\n"
				<< "light position: " << to_string(light.direction) << "\n"
				<< "light color: " << to_string(light.rgb) << "\n"
				<< "modelview : " << to_string(modelview) << "\n"
				//<< "transf : " << to_string(transf) << "\n"
				<< "incoming ray origin: " << to_string(ray_origin) << "\n"
				<< "incoming ray direction: " << to_string(ray_direction) << "\n"
				<< "intersection t value: " << t << "\n"
				<< "intersection point: " << to_string(mypos) << "\n"
				<< "direction from intersection to light: " << to_string(direction) << "\n"
				<< "normed direction from intersection to light: " << to_string(normed_direction) << "\n"
				<< "normalized light+view (halfvec): " << to_string(total_dirn) << "\n"
				<< "normal: " << to_string(normal) << "\n"
				<< "update: " << to_string(update) << "\n"
				<< "t_blocked: " << t_blocked << "\n"
				<< "t_light: " << t_light << "\n"
				<< "recursion depth: " << depth << "\n";
			//getchar();
		}**/
		color += update;
	}

	if (depth < maxdepth) {
		vec4 mirror_direction(ray_direction - 2 * dot(ray_direction, normal)*normal, 0);
		mirror_direction = normalize(mirror_direction);
		ray mirror_ray{mypos4+0.01f*mirror_direction,mirror_direction};
		float t_reflected;
		object o_reflected;
		tie(t_reflected, o_reflected) = intersect(mirror_ray);
		//vec3 reflected_color(0, 0, 0);
		if (t_reflected > 0) {
			return color + (thi.specular * findColor(mirror_ray, t_reflected, o_reflected, depth + 1));
			//color += reflected_color;
		}
	}

	return color;
}

void process(const char* f) {
	readfile(f);

	for (auto& obj : objs) {
		switch (obj.tag) {
		case object::SPHERE: {
			sphere& s = obj.sphere;
			const mat4& transf = s.transform;
			s.mvt = modelview * transf;
			s.mvti = inverse(s.mvt);
			s.mvtit = inverseTranspose(mat3(s.mvt));
		} break;
		case object::TRIANGLE: {
			triangle& tri = obj.triangle;
			const mat4& transf = tri.transform;
			tri.mvt = modelview * transf;
			tri.mvti = inverse(tri.mvt);
			tri.mvtit = inverseTranspose(mat3(tri.mvt));
			// compute vertices/normal camera-coordinates
			tri.v1_t = dehomo(tri.mvt * vec4(tri.v1, 1));
			tri.v2_t = dehomo(tri.mvt * vec4(tri.v2, 1));
			tri.v3_t = dehomo(tri.mvt * vec4(tri.v3, 1));
			tri.n_t = normalize(tri.mvtit * tri.n);

			// precompute intermediate values for barycentric coords
			tri.u0 = tri.v2_t - tri.v1_t;
			tri.u1 = tri.v3_t - tri.v1_t;
			tri.d1 = dot(tri.u0, tri.u0);
			tri.d2 = dot(tri.u0, tri.u1);
			tri.d3 = dot(tri.u1, tri.u1);

			// precompute minimum and maximum x,y,z for basic AABB
			tri.min_x = min(min(tri.v1_t.x, tri.v2_t.x), tri.v3_t.x) - 0.001f;
			tri.min_y = min(min(tri.v1_t.y, tri.v2_t.y), tri.v3_t.y) - 0.001f;
			tri.min_z = min(min(tri.v1_t.z, tri.v2_t.z), tri.v3_t.z) - 0.001f;
			tri.max_x = max(max(tri.v1_t.x, tri.v2_t.x), tri.v3_t.x) + 0.001f;
			tri.max_y = max(max(tri.v1_t.y, tri.v2_t.y), tri.v3_t.y) + 0.001f;
			tri.max_z = max(max(tri.v1_t.z, tri.v2_t.z), tri.v3_t.z) + 0.001f;
		} break;
		}
	}

	// These unused print statements fix scene 5. (?)
	// Hypothesis: Compiler is doing something weird with floating point
	// However, setting fp:strict does not fix it either.
	if (false) {
		for (auto& light : lights) {
			std::cout <<
				"light type: " << (light.type == light::POINT ? "POINT" : "DIRECTIONAL") << "\n"
				<< "light position: " << to_string(light.direction) << "\n"
				<< "light color: " << to_string(light.rgb) << "\n";
		}
	}

	BYTE *pixels = new BYTE[3 * w * h];
	const chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
	const bool track_time = string(f) == "scene7.test";
	//#pragma omp parallel for num_threads(8) schedule(guided)
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			//debug = (i == 221 && j == 79);
			//debug = (i == 140 && j == 200);
			//debug = (i == 390 && j == 390);
			ray ray = rayThruPixel(i, j);
			float hit_t;
			object hit_o;
			tie(hit_t, hit_o) = intersect(ray);
			vec3 color = findColor(ray, hit_t, hit_o, 1);
			int pixelnum = (i*w + j)*3;
			pixels[pixelnum] = min(255, (int)(color[2] * 255));
			pixels[pixelnum + 1] = min(255, (int)(color[1] * 255));
			pixels[pixelnum + 2] = min(255, (int)(color[0] * 255));
		}
		if (track_time) {
			chrono::duration<double> elapsed_seconds = chrono::system_clock::now() - start;
			std::cout << "Finished row " << i + 1 << " at " << elapsed_seconds.count() << " seconds \n";
		}
	}
	FIBITMAP *img = FreeImage_ConvertFromRawBits(pixels, w, h, w * 3, 24, 0x0000FF, 0x00FF00, 0xFF0000, true);

	std::cout << "Saving: " << fname << "\n";

	FreeImage_Save(FIF_PNG, img, fname.c_str(), 0);
	delete pixels;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		cerr << "Usage: transforms scenefile [grader input (optional)]\n";
		exit(-1);
	}

	FreeImage_Initialise();
	//process(argv[1]);
	//process("scene4-ambient.test");
	//process("scene4-diffuse.test");
	//process("scene4-emission.test");
	//process("scene4-specular.test");
	process("scene5.test");
	//process("scene6.test");
	//process("scene7.test");
	FreeImage_DeInitialise();
	getchar();
	return 0;
}