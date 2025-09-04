#include <iostream>
#include <time.h>
#include <math.h>

#include "render.hh"
#include "world_setup.hh"
#include "input.hh"

/*
 * Controls:
 * Camera up: W
 * Camera down: S
 * Camera left: A
 * Camera right: D
 * Rotate camera left/right: E/R
 * Rotate camera up/down: F/V
 */

int main(int argc, char *argv[])
{

	/* These should be even for convenience */
	int screen_x = 56;
	int screen_y = 24;
	Renderer renderer(screen_y, screen_x, 3.0, 100);

	setup(&renderer);
	input_setup();

	double theta = 0.0;
	double omega = 0.02;
	double tetra_base_y = 60.0;
	double turn_radius = 30.0;
	while (1) {
		//handle_input(&renderer);
		renderer.camera_xangle -= omega;
		theta -= omega;
		renderer.camera_pos[1] = tetra_base_y - turn_radius * cos(theta);
		renderer.camera_pos[0] = turn_radius * sin(theta);
		
		renderer.updated = true;
		if (renderer.updated) {
			/* used to be *** */
			Render_symbol **a = renderer.render();
				for (int i = 0; i < screen_y; ++i)
				std::cout << "\n";
				for (int i = 0; i < screen_y; ++i) {
					for (int j = 0; j < screen_x; ++j) {
/* used to be *a */
						std::cout << (a)[i][j].get_string();
					}
					std::cout << "\n";
				}
		}
		/* Limit framerate. Still doesn't play nicely on slow terminals */
		//struct timespec u = {.tv_sec = 0, .tv_nsec = 1670000l};
		struct timespec u = {.tv_sec = 0, .tv_nsec = 1000000000l / 10l};
		nanosleep (&u, 0);

	}


	return 0;
}
