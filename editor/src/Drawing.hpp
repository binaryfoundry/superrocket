#pragma once

#include "Math.hpp"

#include <SDL.h>

extern int window_width, window_height;
extern SDL_Renderer* renderer;

extern vec3 view_position;
extern float view_fov;
extern float view_near_z;

extern mat4x4 projection;
extern mat4x4 view;
extern mat4x4 projection_view;

template <typename T>
T lerp(T v0, T v1, float t)
{
    return v0 * (1 - t) + v1 * t;
}

float plane_distance(
    vec3 plane_n,
    vec3 plane_d,
    vec3 p);

void draw_point_3d(
    vec3 point,
    float size);

void draw_line_3d(
    vec3 l0,
    vec3 l1);

void draw_circle(
    Sint16 x,
    Sint16 y,
    Sint16 rad,
    Uint8 r,
    Uint8 g,
    Uint8 b);

vec4 project_screen(
    vec4 p);

vec3 unproject(
    vec2 screen_position);

bool picking_raycast(
    vec3& position,
    vec3 plane_n,
    vec3 plane_d,
    int16_t screen_x,
    int16_t screen_y);
