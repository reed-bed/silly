#ifndef INPUT_H_I
#define INPUT_H_I

#include "render.hh"

/* sets up terminal for input */
void input_setup();

/* Provides non-blocking input handling */
void handle_input(Renderer *renderer);

#endif
