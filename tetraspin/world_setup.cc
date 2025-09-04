#include <math.h>
#include <iostream>
#include "world_setup.hh"
#include "vec_ops.hh"

void setup(Renderer *renderer)
{

	Render_object obj;
	/* Draw a regular tetrahedron */
	
	//double length = 15.0;
	double length = 20.0;
	std::vector<double> base({0.0, 60.0, -8.0});
	std::vector<double> vertices[4] = {{0.0, -1.0, 0.0},
									   {-cos(M_PI/6), sin(M_PI/6), 0.0},
									   {cos(M_PI/6), sin(M_PI/6), 0.0},
									   {0.0, 0.0, sqrt(2.0/3.0)}};
	for (int i = 0; i < 4; ++i) {
		vertices[i] = vec_add(vec_mult(vertices[i], length), base);
	}
	for (int i = 0; i < 4; ++i) {
		for (int j = i + 1; j < 4; ++j) {
			for (int k = j + 1; k < 4; ++k) {
				std::vector<double> p[3] = {vertices[i], vertices[j],
					vertices[k]};
				obj.add_triangle(p, '-', fg_red, bg_none, '*', fg_none, bg_white);
			}
		}
	}


	renderer->add_object(obj);
	return;

}
