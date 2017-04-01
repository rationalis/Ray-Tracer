/*****************************************************************************/
/* This is the program skeleton for homework 2 in CSE167 by Ravi Ramamoorthi */
/* Extends HW 1 to deal with shading, more transforms and multiple objects   */
/*****************************************************************************/

/*****************************************************************************/
// This file is readfile.cpp.  It includes helper functions for matrix 
// transformations for a stack (matransform) and to rightmultiply the 
// top of a stack.  These functions are given to aid in setting up the 
// transformations properly, and to use glm functions in the right way.  
// Their use is optional in your program.  
  

// The functions readvals and readfile do basic parsing.  You can of course 
// rewrite the parser as you wish, but we think this basic form might be 
// useful to you.  It is a very simple parser.
  
/*****************************************************************************/

// Basic includes to get this file to work.  
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <stack>
#include "Transform.h" 

using namespace std;
#include "variables.h" 
#include "readfile.h"

void rightmultiply(const mat4 & M, stack<mat4> &transfstack) 
{
    mat4 &T = transfstack.top(); 
    T = T * M; 
}

// Function to read the input data values
// Use is optional, but should be very helpful in parsing.  
bool readvals(stringstream &s, const int numvals, float* values) 
{
    for (int i = 0; i < numvals; i++) {
        s >> values[i]; 
        if (s.fail()) {
            cout << "Failed reading value " << i << " will skip\n"; 
            return false;
        }
    }
    return true; 
}

void readfile(const char* filename) 
{
	// default initialize things
	maxdepth = 5;
	fname = "test000.png";
	const_attenuation = 1;
	linear_attenuation = 0;
	quadratic_attenuation = 0;

	ambient[0] = 0.2f;
	ambient[1] = 0.2f;
	ambient[2] = 0.2f;

    string str, cmd; 
    ifstream in;
    in.open(filename); 
    if (in.is_open()) {

        // I need to implement a matrix stack to store transforms.  
        // This is done using standard STL Templates 
        stack <mat4> transfstack; 
        transfstack.push(mat4(1.0));  // identity

        getline (in, str); 
        while (in) {
            if ((str.find_first_not_of(" \t\r\n") != string::npos) && (str[0] != '#')) {
                // Ruled out comment and blank lines 

                stringstream s(str);
                s >> cmd; 
                int i; 
                float values[10]; // Position and color for light, colors for others
                bool validinput; // Validity of input

				if (cmd == "maxdepth") {
					validinput = readvals(s, 1, values);
					if (validinput) {
						maxdepth = (int)values[0];
					}
				}
				else if (cmd == "output") {
					s >> fname;
				}
				else if (cmd == "point") {
					validinput = readvals(s, 6, values);
					if (validinput) {
						lights.push_back(light{
							light::POINT,
							dehomo(modelview * transfstack.top() * vec4(values[0],values[1],values[2],1)),
							vec3(values[3],values[4],values[5]),
							const_attenuation,
							linear_attenuation,
							quadratic_attenuation
							//transfstack.top()
						});
					}
				}
				else if (cmd == "directional") {
					validinput = readvals(s, 6, values);
					if (validinput) {
						lights.push_back(light{
							light::DIRECTIONAL,
							dehomo(modelview * transfstack.top() * vec4(values[0],values[1],values[2],0)),
							vec3(values[3],values[4],values[5]),
							1,
							0,
							0
							//transfstack.top()
						});
					}
					vec3 v = dehomo(vec4(0, 0, 0, 0));
				}
				else if (cmd == "attenuation") {
					validinput = readvals(s, 3, values);
					if (validinput) {
						const_attenuation = values[0];
						linear_attenuation = values[1];
						quadratic_attenuation = values[2];
					}
				}
				else if (cmd == "vertex") {
					validinput = readvals(s, 3, values);
					if (validinput) {
						verts.push_back(vec3(values[0], values[1], values[2]));
					}
				}
				else if (cmd == "tri") {
					validinput = readvals(s, 3, values);
					if (validinput) {
						triangle t;

						// Set the object's light properties
						for (i = 0; i < 3; i++) {
							(t.ambient)[i] = ambient[i];
							(t.diffuse)[i] = diffuse[i];
							(t.specular)[i] = specular[i];
							(t.emission)[i] = emission[i];
						}
						t.shininess = shininess;

						// Set the object's transform
						t.transform = transfstack.top();

						t.v1 = verts[(int)values[0]];
						t.v2 = verts[(int)values[1]];
						t.v3 = verts[(int)values[2]];
						t.n = normalize(cross(t.v2 - t.v1, t.v3 - t.v1));

						object obj;
						obj.tag = object::TRIANGLE;
						obj.triangle = t;
						objs.push_back(obj);
					}
				}

                // Material Commands 
                // Ambient, diffuse, specular, shininess properties for each object.
                // Filling this in is pretty straightforward, so I've left it in 
                // the skeleton, also as a hint of how to do the more complex ones.
                // Note that no transforms/stacks are applied to the colors. 

                else if (cmd == "ambient") {
                    validinput = readvals(s, 3, values); // colors 
                    if (validinput) {
                        for (i = 0; i < 3; i++) {
                            ambient[i] = values[i]; 
                        }
                    }
                } else if (cmd == "diffuse") {
                    validinput = readvals(s, 3, values); 
                    if (validinput) {
                        for (i = 0; i < 3; i++) {
                            diffuse[i] = values[i]; 
                        }
                    }
                } else if (cmd == "specular") {
                    validinput = readvals(s, 3, values); 
                    if (validinput) {
                        for (i = 0; i < 3; i++) {
                            specular[i] = values[i]; 
                        }
                    }
                } else if (cmd == "emission") {
                    validinput = readvals(s, 3, values); 
                    if (validinput) {
                        for (i = 0; i < 3; i++) {
                            emission[i] = values[i]; 
                        }
                    }
                } else if (cmd == "shininess") {
                    validinput = readvals(s, 1, values); 
                    if (validinput) {
                        shininess = values[0]; 
                    }
                } else if (cmd == "size") {
                    validinput = readvals(s,2,values); 
                    if (validinput) { 
                        w = (int) values[0]; h = (int) values[1]; 
                    } 
                } else if (cmd == "camera") {
                    validinput = readvals(s,10,values); // 10 values eye cen up fov
                    if (validinput) {
						for (int i = 0; i < 3; i++) {
							eyeinit[i] = values[i];
							center[i] = values[i + 3];
							upinit[i] = values[i + 6];
						}
						fovy = values[9];

						modelview = Transform::lookAt(
							eyeinit, center,
							Transform::upvector(upinit, eyeinit-center));
                    }
                }
				else if (cmd == "sphere") {
					validinput = readvals(s, 4, values);
					if (validinput) {
						sphere s;

						// Set the object's light properties
						for (i = 0; i < 3; i++) {
							(s.ambient)[i] = ambient[i];
							(s.diffuse)[i] = diffuse[i];
							(s.specular)[i] = specular[i];
							(s.emission)[i] = emission[i];
						}
						s.shininess = shininess;

						// Set the object's transform
						s.transform = transfstack.top();

						s.center = vec3(values[0], values[1], values[2]);
						s.radius = values[3];

						object obj;
						obj.tag = object::SPHERE;
						obj.sphere = s;
						objs.push_back(obj);
					}
				}

                else if (cmd == "translate") {
                    validinput = readvals(s,3,values); 
                    if (validinput) {
						rightmultiply(Transform::translate(values[0],values[1],values[2]), transfstack);
                    }
                }
                else if (cmd == "scale") {
                    validinput = readvals(s,3,values); 
                    if (validinput) {
						rightmultiply(Transform::scale(values[0], values[1], values[2]), transfstack);
                    }
                }
                else if (cmd == "rotate") {
                    validinput = readvals(s,4,values); 
                    if (validinput) {
						rightmultiply(mat4(Transform::rotate(values[3], vec3(values[0], values[1], values[2]))), transfstack);
                    }
                }

                // I include the basic push/pop code for matrix stacks
                else if (cmd == "pushTransform") {
                    transfstack.push(transfstack.top()); 
                } else if (cmd == "popTransform") {
                    if (transfstack.size() <= 1) {
                        cerr << "Stack has no elements.  Cannot Pop\n"; 
                    } else {
                        transfstack.pop(); 
                    }
                }

                //else {
                //    cerr << "Unknown Command: " << cmd << " Skipping \n"; 
                //}
            }
            getline (in, str); 
        }

        // Set up initial position for eye, up
        eye = eyeinit; 
        up = Transform::upvector(upinit, eye-center);
    } else {
        cerr << "Unable to Open Input Data File " << filename << "\n"; 
        throw 2; 
    }
}
