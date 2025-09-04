#include <stdlib.h>
#include <cstddef>
#include <limits>
#include <math.h>
#include <algorithm>


#include "render.hh"
#include "vec_ops.hh"


Render_symbol::Render_symbol(char c, enum fg_colour f,
		enum bg_colour b)
{
	character = c;
	fg = f;
	bg = b;
	/* Everything should overwrite an unwritten tile */
	distance = std::numeric_limits<double>::infinity();
}

std::string Render_symbol::get_string()
{
	/*
	 * Can get away with e.g. \x1b[0;41m, but not \x1b[41;0m as a colour
	 * code. The brackets *are* important; otherwise, e.g. 'm' and
	 * this->character get added together
	 */
	return (((((std::string("\x1b[") + std::to_string(this->fg)) +
		(this->bg ? std::string(";") + std::to_string(this->bg) : "")) + 'm') +
		this->character) + "\x1b[0m");
}

/*
 * Need to specify that local variables line, fill should be created with
 * auto-generated copy constructors, apparently
 */
Triangle::Triangle(std::vector<double> v[3], Render_symbol line,
		Render_symbol fill) : line_symbol(line), fill_symbol(fill)
{
	/* Copy v into list of vertices.*/
	for (int i = 0; i < 3; ++i) {
		this->vertices[i] = std::vector<double>(v[i]);
	}

}

Render_object::Render_object()
{
	this->triangles = std::vector<Triangle>();
}

void Render_object::add_triangle(std::vector<double> verts[3], char line_c,
		enum fg_colour line_fg, enum bg_colour line_bg,
		char fill_c, enum fg_colour fill_fg,
		enum bg_colour fill_bg)
{
	Render_symbol sym_line(line_c, line_fg, line_bg);
	Render_symbol sym_fill(fill_c, fill_fg, fill_bg);
	this->triangles.emplace_back(Triangle(verts, sym_line, sym_fill));
}

Renderer::Renderer(int scr_y, int scr_x, double c_d, int max_objects)
	: camera_pos(3, 0)
{
	this->max_objects = max_objects;
	this->n_objects = 0;
	this->render_objects = (Render_object *) calloc(max_objects,
			sizeof(Render_object));

	this->camera_xangle = 0.0;
	this->camera_zangle = 0.0;
	this->screen_x = scr_x;
	this->screen_y = scr_y;
	this->camera_depth = c_d;

	this->updated = 1;

	this->screen = (Render_symbol **) calloc(sizeof(Render_symbol *), scr_y);
	for(int i = 0; i < scr_y; ++i)
		screen[i] = (Render_symbol *) calloc(sizeof(Render_symbol), scr_x);

}

Render_object *Renderer::add_object(Render_object obj)
{
	/* Returns 0 on failure */
	if (this->n_objects - this->max_objects) {
		int n = this->n_objects;
		this->render_objects[n_objects++] = obj;
		return &(render_objects[n]);
	}
	return 0;

}

/* The return value of this function should not be modified. */
/* used to be *** */
Render_symbol **Renderer::render()
{
	/* First, clear screen */
	for (int i = 0; i < this->screen_y; ++i) {
		for (int j = 0; j < this->screen_x; ++j) {
			this->screen[i][j] = Render_symbol(' ', fg_none, bg_none);
		}
	}

	/* Iterate through objects and draw them */
	Render_object *render_objs = this->render_objects;
	/* Not calling size every iteration of a loop is probably good */
	int n_objs = this->n_objects;
	for (int i = 0; i < n_objs; ++i) {
		std::vector<Triangle> triangles = render_objs[i].triangles;
		size_t n_tria = triangles.size();
		for(size_t j = 0; j < n_tria; ++j) {
			this->draw_triangle(&triangles[j]);
		}
	}
	this->updated = 0;
	/* used to be &(...) */
	return (this->screen);
}

void Renderer::draw_triangle(Triangle *t)
{
	/*
	 * 1: find on-screen coordinates and distances from screen of triangle
	 * vertices
	 */

	std::vector<double> *scs = this->screen_project(t);
	if (!scs)
		return;

	/* Triangles are thin and cannot be seen side-on */
	for (int i = 0; i < 3; ++i) {
		for (int j = i+1; j <3; ++j) {
			if ((scs[i][0] - scs[j][0]) || (scs[i][1] - scs[j][1]))
				continue;
			return;
		}
	}

	/*
	 * 2: shade in area bounded by all cells intersected by edges of triangle,
	 * recording approximate distance from each cell to triangle (this is not a
	 * serious program, so a little laziness is excusable).
	 */
	/*
	 * Coordinates are relative to centre of screen in scs, but need to convert
	 * to characters (8pxX15px on my terminal) to write to screen. Multiply by
	 * screen_x/2.5 (for x) (5 unit wide camera plane) or (8/15)*screen_x/(2.5)
	 * (for y)
	 */
	double ys = 15.0/8.0;
	double scr_x_c = this->screen_x / 5.0;
	double scr_y_c = scr_x_c / ys;

	for (int i = 0; i < 3; ++i) {
		scs[i][1] *= scr_x_c;
		scs[i][0] *= scr_y_c;
	}


	double ymin = floor(std::min(scs[0][0], std::min(scs[1][0], scs[2][0])));
	double ymax = ceil(std::max(scs[0][0], std::max(scs[1][0], scs[2][0])));
	double xmin = floor(std::min(scs[0][1], std::min(scs[1][1], scs[2][1])));
	double xmax = ceil(std::max(scs[0][1], std::max(scs[1][1], scs[2][1])));

	/* Use convex combination trickery to shade triangle only */
	double dist0 = scs[0][2];
	double dc1 = scs[1][2] - dist0;
	double dc2 = scs[2][2] - dist0;

	std::vector<double> r0({scs[0][0], scs[0][1]});

	std::vector<double> u1 = vec_sub(scs[1], scs[0]);
	u1.pop_back();
	std::vector<double> u2 = vec_sub(scs[2], scs[0]);
	u2.pop_back();


	/* Features corrections to avoid drawing off-screen */
	for (int y = (int) std::max(-1.0*screen_y/2, floor(ymin));
			y <= std::min(1.0*screen_y/2 - 1.0, ceil(ymax)); ++y) {

		for (int x = (int) std::max(-1.0*screen_x/2, floor(xmin));
				x <= std::min(1.0*screen_x/2 - 1.0, ceil(xmax)); ++x) {

			/* Map coordinates to array indices */
			int yp = screen_y/2 - 1 - y;
			int xp = x + screen_x/2;
			double dist = this->dist_calc(r0, u1, u2, dist0, dc1, dc2, y, x);
			/* Nearer things cover farther ones */
			if (dist >= screen[yp][xp].distance) {
				continue;
			}
			if (this->is_inside(r0, u1, u2, y, x)) {
				/* Check if this is at the edge of a triangle */
				int edge = 0;
				for (int p = -1; p <= 1 && !edge; p+=2) {
					for (int q = -1; q <=1 && !edge; q+=2) {
						if (!(this->is_inside(r0, u1, u2, y+p, x+q))) {
							this->screen[yp][xp] = Render_symbol(t->line_symbol);
							this->screen[yp][xp].distance = dist;
							edge = 1;
						}
					}
				}
				if (edge)
					continue;
				/* If this isn't the edge of a triangle, draw accordingly */
				this->screen[yp][xp] = Render_symbol(t->fill_symbol);
				this->screen[yp][xp].distance = dist;
			}
		}
	}
}
std::vector<double> *Renderer::screen_project(Triangle *t)
{
	/*
	 * Return value is pointer to static variable to avoid repeated allocations;
	 * don't need result of using this on two triangles at the same time, so
	 * no problem. Returns 0 if line from a vertex to camera never intersects
	 * camera plane; that's not a problem as in that case, the triangle is
	 * either behind the plane or intersects the camera; either way, this is
	 * not normal.
	 */

	/* Components: screen x coordinate, screen y coordinate, distance from scr */
	static std::vector<double> ret[3];
	
	double d = this->camera_depth;
	double cx = this->camera_xangle;
	double cz = this->camera_zangle;
	double a, b, l, q;
	std::vector<double> C0 = this->camera_pos;
	std::vector<double> Cr({cos(cx), sin(cx), 0});
	std::vector<double> Cu({sin(-cz)*sin(cx), sin(cz)*cos(cx), cos(cz)});
	std::vector<double> Cf = vec_cross(Cu, Cr);

	for (int i = 0; i < 3; ++i) {
		/* I worked this out on paper; explaining it in comments would be a pain */
		std::vector<double> P = t->vertices[i];
		std::vector<double> A = vec_sub(P, C0);
		q = vec_dot(A, Cf);

		if (!q or (vec_dot(Cf, A) < 0))
			return 0;

		l = d/q;
		b = vec_dot(A, Cr) * l;
		a = vec_dot(A, Cu) * l;

		ret[i] = std::vector<double>({a, b, vec_dot(vec_sub(C0, P),
					vec_sub(C0, P))});
	}
	return ret;
}

int Renderer::is_inside(std::vector<double> r0, std::vector<double> u1,
		std::vector<double> u2, int y, int x)
{
	/*
	 * These are guaranteed to exist because we earlier eliminated the case 
	 * where any of scs[0] [1] or [2] have equal [0] and [1] components
	 */
	std::vector<double> v1 = vec_recip2(u1, u2);
	std::vector<double> v2 = vec_recip2(u2, u1);

	std::vector<double> r = vec_sub(std::vector<double>({y*1.0, x*1.0}),
			r0);
	double c1 = vec_dot(r, v1);
	double c2 = vec_dot(r, v2);
	return (c1 <= 1.0 && c1 >= 0.0 && c2 <=1.0 && c2 >= 0.0 && (c1 + c2) <= 1.0);
}


double Renderer::dist_calc(std::vector<double> r0, std::vector<double> u1,
		std::vector<double> u2, double dist0, double dc1, double dc2,
		int y, int x)
{
	std::vector<double> v1 = vec_recip2(u1, u2);
	std::vector<double> v2 = vec_recip2(u2, u1);
	std::vector<double> r = vec_sub(std::vector<double>({y*1.0, x*1.0}), r0);
	return (dist0 + dc1*vec_dot(r, v1) + dc2*vec_dot(r, v2));

}
