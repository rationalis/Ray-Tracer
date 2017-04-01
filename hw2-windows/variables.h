/*****************************************************************************/
/* This is the program skeleton for homework 2 in CSE167 by Ravi Ramamoorthi */
/* Extends HW 1 to deal with shading, more transforms and multiple objects   */
/*****************************************************************************/

#pragma once

#include <string>
#include <vector>


// This is the basic include file for the global variables in the program.  
// Since all files need access to it, we define EXTERN as either blank or 
// extern, depending on if included in the main program or not.  

#ifdef MAINPROGRAM 
#define EXTERN 
#else 
#define EXTERN extern
#endif

static vec3 dehomo(const vec4& v) { return vec3(v) / (abs(v.w) < 0.000001f ? 1 : v.w); }

typedef struct { vec4 origin; vec4 direction; } ray;

typedef struct thing {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 emission;
	float shininess;
	mat4 transform;

	mat4 mvt;
	mat4 mvti;
	mat3 mvtit;
} thing;
typedef struct sphere : thing { vec3 center; float radius; } sphere;

typedef struct vertnorm {
	vec3 vertex;
	vec3 normal;
} vertnorm;

EXTERN std::vector<vec3> verts;
EXTERN std::vector<vertnorm> vertnorms;

typedef struct triangle : thing {
	vec3 v1,v2,v3,n,v1_t,v2_t,v3_t,n_t,u0,u1;
	float d1, d2, d3, min_x, max_x, min_y, max_y, min_z, max_z;
} triangle;
//typedef struct normal_triangle : triangle { vec3 n1; vec3 n2; vec3 n3; } normal_triangle;

typedef struct object {
	object() : tag(SPHERE), sphere() {}
	enum { SPHERE, TRIANGLE, NORMAL_TRIANGLE } tag;
	union {
		sphere sphere;
		triangle triangle;
		//normal_triangle normal_triangle;
	};
} object;

EXTERN std::vector<object> objs;

EXTERN float const_attenuation;
EXTERN float linear_attenuation;
EXTERN float quadratic_attenuation;

typedef struct light {
	enum {DIRECTIONAL, POINT} type;
	vec3 direction;
	vec3 rgb;
	float const_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
	//mat4 transform;
} light;

EXTERN std::vector<light> lights;

EXTERN int maxdepth;
EXTERN std::string fname;


// ----------------------------------------------------------------------------------

EXTERN vec3 eye;
EXTERN vec3 up;

#ifdef MAINPROGRAM 
vec3 eyeinit(0.0,0.0,5.0) ; // Initial eye position, also for resets
vec3 upinit(0.0,1.0,0.0) ; // Initial up position, also for resets
vec3 center(0.0,0.0,0.0) ; // Center look at point 
int amountinit = 5;
int w = 500, h = 500 ; // width and height 
float fovy = 90.0 ; // For field of view
#else 
EXTERN vec3 eyeinit ; 
EXTERN vec3 upinit ; 
EXTERN vec3 center ; 
EXTERN int amountinit;
EXTERN int w, h ; 
EXTERN float fovy ; 
#endif 

EXTERN mat4 modelview;

// Materials (read from file) 
// With multiple objects, these are colors for each.
EXTERN float ambient[3] ; 
EXTERN float diffuse[3] ; 
EXTERN float specular[3] ; 
EXTERN float emission[3] ; 
EXTERN float shininess ;
