#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include "input.hh"
#include "vec_ops.hh"

static void move_lateral(Renderer *rend, double distance);
static void move_vertical(Renderer *rend, double distance);
static void move_forward_backward(Renderer *rend, double distance);

void input_setup()
{
	struct termios t_conf;
	tcgetattr(0, &t_conf);
	/* Disable canonical mode to read input character by character */
	t_conf.c_lflag &= ~ICANON;
	/* Disable input echoing */
	t_conf.c_lflag &= ~ECHO;
	/* Minimum read size: zero characters (allows non-blocking read) */
	t_conf.c_cc[VMIN] = 0;
	/* Reading times out immediately */
	t_conf.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &t_conf);
}


void handle_input(Renderer *renderer)
{
	/* Check for user input */
	char in = 0;
   	read(0, &in, 1);
	while (in) {
		renderer->updated = true;
		switch (in) {
			case 'a':
				move_lateral(renderer, -0.2);
				break;
			case 'd':
				move_lateral(renderer, 0.2);
				break;
			case 'w':
				move_vertical(renderer, 0.2);
				break;
			case 's':
				move_vertical(renderer, -0.2);
				break;
			case 'q':
				move_forward_backward(renderer, 0.2);
				break;
			case 'z':
				move_forward_backward(renderer, -0.2);
				break;
			case 'e':
				renderer->camera_xangle += 0.02;
				break;
			case 'r':
				renderer->camera_xangle -= 0.02;
				break;
			case 'f':
				renderer->camera_zangle -= 0.02;
				break;
			case 'v':
				renderer->camera_zangle += 0.02;
				break;

		}

		in = 0;
		read(0, &in, 1);
	}
}


static void move_lateral(Renderer *rend, double distance)
{
	std::vector<double> lat_vec({cos(rend->camera_xangle),
			sin(rend->camera_xangle), 0});
	lat_vec = vec_mult(lat_vec, distance);
	rend->camera_pos = vec_add(rend->camera_pos, lat_vec);
}

static void move_forward_backward(Renderer *rend, double distance)
{
	std::vector<double> fb_vec({-1.0 * sin(rend->camera_xangle),
			cos(rend->camera_xangle), 0});
	fb_vec = vec_mult(fb_vec, distance);
	rend->camera_pos = vec_add(rend->camera_pos, fb_vec);
}

static void move_vertical(Renderer *rend, double distance)
{
	std::vector<double> vert_vec({-sin(rend->camera_zangle) *
			sin(rend->camera_xangle),
			sin(rend->camera_zangle) * cos(rend->camera_xangle),
			cos(rend->camera_zangle)});

	vert_vec = vec_mult(vert_vec, distance);
	rend->camera_pos = vec_add(rend->camera_pos, vert_vec);
}
