#pragma once

#include "../field.h"


void display_grid_init();
void display_grid_deinit();
void display_grid(Field *field, int frame_n);

static const int fps = 25;
static const int animation_frames_n = 5;
static const int scale = 4;
static const int source_sprite_size = 16;
static const int total_sprite_size = scale * source_sprite_size;
