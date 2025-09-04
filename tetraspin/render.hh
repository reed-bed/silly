#ifndef RENDER_H_I
#define RENDER_H_I

#include <string>
#include <vector>

/* These are the colour codes for ANSI terminal colours */
enum fg_colour {
	fg_none = 0,
	fg_black = 30,
	fg_red = 31,
	fg_green = 32,
	fg_yellow = 33,
	fg_blue = 34,
	fg_magenta = 35,
	fg_cyan = 36,
	fg_white = 37,
	/* Bright colours */
	fg_b_black = 90,
	fg_b_red = 91,
	fg_b_green = 92,
	fg_b_yellow = 93,
	fg_b_blue = 94,
	fg_b_magenta = 95,
	fg_b_cyan = 96,
	fg_b_white = 97
};

enum bg_colour {
	bg_none = 0,
	bg_black = 40,
	bg_red = 41,
	bg_green = 42,
	bg_yellow = 43,
	bg_blue = 44,
	bg_magenta = 45,
	bg_cyan = 46,
	bg_white = 47,
	/* Bright colours */
	bg_b_black = 100,
	bg_b_red = 101,
	bg_b_green = 102,
	bg_b_yellow = 103,
	bg_b_blue = 104,
	bg_b_magenta = 105,
	bg_b_cyan = 106,
	bg_b_white = 107
};

/* Describes what to draw on the screen when a shape is rendered */
class Render_symbol {
	public:
		Render_symbol(char c, enum fg_colour f,	enum bg_colour b);
		std::string get_string();
		/*
		 * For use in Renderer.render() to check whether to overwrite a symbol
		 * because it is covered by something nearer
		 */
		double distance;
	private:
		enum fg_colour fg;
		enum bg_colour bg;
		char character;
};

/*
 * Triangle would look almost exactly the same as a struct, but I'm trying to
 * write something that looks like C++ and not C.
 * Triangles are used to make 3D shapes for rendering.
 */
class Triangle {
	public:
		Triangle(std::vector<double> v[3], Render_symbol line, Render_symbol fill);
		std::vector<double> vertices[3];
		Render_symbol line_symbol;
		Render_symbol fill_symbol;
};

/* Describes a 3D shape */
class Render_object {
	public:
		Render_object();
		std::vector<Triangle> triangles;
		void add_triangle(std::vector<double> verts[3], char line_c = '*',
				enum fg_colour line_fg = fg_none,
				enum bg_colour line_bg = bg_none, char fill_c = ' ',
				enum fg_colour fill_fg = fg_none,
				enum bg_colour fill_bg = bg_none);
};

/*
 * Contains camera information, the list of things to render, and the array of
 * Render_symbols resulting from 3D rendering. Arguments are screen dimensions
 * (in characters), and distance from back of camera to plane
 * (render() works by intersecting lines (from the point at the back of the
 * camera to objects) with a plane, so depth determines field of vision with a 
 * fixed screen width of 5.0)
 */
class Renderer {
	public:
		Renderer(int scr_y, int scr_x, double c_depth, int max_objects);
		/* Returns a pointer to added object in case it needs modifying */
		Render_object *add_object(Render_object obj);
		/*
		 * render() returns a reference to screen, the array of Render_symbols
		 * produced
		 */
		/* used to be *** */
		Render_symbol **render();
		/*
		 * Array is more suitable than a vector here: lacks dynamic
		 * reallocation, but pointers don't spontaneously get invalidated
		 */
		Render_object *render_objects;
		int screen_x, screen_y;
		int updated;

		std::vector<double> camera_pos;
		/* Camera angle to x-axis in xy plane*/
		double camera_xangle;
		/* Camera angle to z-axis */
		double camera_zangle;
	private:
		/* Takes a reference to avoid copying argument */
		void draw_triangle(Triangle *t);
		std::vector<double> *screen_project(Triangle *t);
		/*
		 * Works out if a point (y, x) is inside the triangle spanned by u1 and
		 * u2 with one vertex at r0
		 */
		int is_inside(std::vector<double> r0, std::vector<double> u1,
				std::vector<double> u2, int y, int x);

		/*
		 * Calculates the distance (from the back of the camera) of the point
		 * (y, x) of a triangle with a vertex at r0 and spanned by u1, u2
		 * with the other vertices a distance dc1 or dc2 further from the origin
		 * than r0, given the distance dist0 of r0 from the origin.
		 */
		double dist_calc(std::vector<double> r0, std::vector<double> u1,
				std::vector<double> u2, double dist0, double dc1, double dc2,
				int y, int x);

		double camera_depth;
		/*
		 * Screen will not be in a valid state until render() has been invoked
		 * at least once. Screen memory is allocated when Renderer is
		 * constructed, and there is no mechanism to free it because Renderer
		 * is supposed to be constantly in use.
		 * Entries are ordered (column, row), with (0, 0) in top left-hand
		 * corner of screen, as that is convenient for output.
		 */
		Render_symbol **screen;
		int n_objects, max_objects;
};

#endif
