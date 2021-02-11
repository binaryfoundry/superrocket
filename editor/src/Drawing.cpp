#include "Drawing.hpp"

#include <SDL2_gfxPrimitives.h>

SDL_Renderer* renderer = nullptr;

int window_width = 0;
int window_height = 0;

vec3 view_position = vec3(0, 1, 0);
float view_fov = 90.0f;
float view_near_z = 0.1f;

vec3 near_plane_n = vec3(0.0, 0.0, 1);
vec3 near_plane_d = vec3(0.0, 0.0, -view_near_z);

mat4x4 projection;
mat4x4 view;
mat4x4 projection_view;

float near_plane_distance(
    vec3 p)
{
    return glm::dot(p - near_plane_d, near_plane_n);
}

vec4 near_plane_line_clip(
    vec4 p0,
    vec4 p1,
    float d0,
    float d1)
{
    return lerp(p0, p1, d0 / (d0 - d1));
}

float plane_distance(
    vec3 plane_n,
    vec3 plane_d,
    vec3 p)
{
    return glm::dot(p - plane_d, plane_n);
}

vec3 plane_line_clip(
    vec4 plane_n, vec4 plane_d,
    vec4 p0, vec4 p1,
    bool& no_hit)
{
    no_hit = false;
    float d0 = plane_distance(
        plane_n,
        plane_d, p0);

    float d1 = plane_distance(
        plane_n,
        plane_d, p1);

    if (d1 > 0)
    {
        no_hit = true;
        return vec3(0, 0, 0);
    }
    return lerp(p0, p1, d0 / (abs(d0) + abs(d1)));
}

void draw_line_segment(
    float x0,
    float y0,
    float x1,
    float y1)
{
    SDL_RenderDrawLine(renderer,
        static_cast<int>(x0), static_cast<int>(y0),
        static_cast<int>(x1), static_cast<int>(y1));
}

void draw_point_3d(
    vec3 point,
    float size)
{
    Uint8 r, g, b, a;

    SDL_GetRenderDrawColor(renderer,
        &r, &g, &b, &a);

    vec4 p = projection_view * vec4(point, 1.0f);
    p = project_screen(p);
    if (p.z > -view_near_z)
    {
        int rad = static_cast<int>(size / p.w);
        rad = rad < 3 ? 3 : rad;
        filledCircleRGBA(
            renderer,
            static_cast<Sint16>(p.x),
            static_cast<Sint16>(p.y),
            rad,
            r, g, b, a);
    }
}

void draw_line_3d(
    vec3 l0,
    vec3 l1)
{
    vec4 p0 = projection_view * vec4(l0, 1.0f);
    vec4 p1 = projection_view * vec4(l1, 1.0f);
    float d0 = near_plane_distance(p0);
    float d1 = near_plane_distance(p1);
    if (d0 < 0 && d1 < 0)
    {
        return;
    }

    if (d0 < 0)
    {
        p0 = near_plane_line_clip(p0, p1, d0, d1);
    }
    else if (d1 < 0)
    {
        p1 = near_plane_line_clip(p1, p0, d1, d0);
    }

    p0 = project_screen(p0);
    p1 = project_screen(p1);
    draw_line_segment(p0.x, p0.y, p1.x, p1.y);
}

void draw_circle(
    Sint16 x,
    Sint16 y,
    Sint16 rad,
    Uint8 r,
    Uint8 g,
    Uint8 b)
{
    circleRGBA(
        renderer,
        x,
        y,
        rad,
        r, g, b, 255);
}

vec4 project_screen(
    vec4 p)
{
    float d = 1.0f / p.w;
    p.x = (p.x * d) * window_width;
    p.y = (p.y * d) * window_height;
    p.x += window_width / 2.0f;
    p.y += window_height / 2.0f;
    return p;
}

vec3 unproject(
    vec2 screen_position)
{
    screen_position = (screen_position * 2.0f) - 1.0f;
    screen_position *= 0.5;

    glm::mat4 invVP = glm::inverse(
        projection_view);

    glm::vec4 screenPos = glm::vec4(
        screen_position.x,
        screen_position.y,
        1.0f,
        1.0f);
    glm::vec4 worldPos = invVP * screenPos;

    glm::vec3 dir = glm::vec3(worldPos);

    return dir;
}

bool picking_raycast(
    vec3& position,
    vec3 plane_n,
    vec3 plane_d,
    int16_t screen_x,
    int16_t screen_y)
{
    position = vec3(0, 0, 0);

    vec2 normalized_mouse(
        static_cast<float>(screen_x) / window_width,
        static_cast<float>(screen_y) / window_height);

    vec3 ray_end = unproject(
        normalized_mouse);

    ray_end = ray_end * 100.0f;

    bool no_hit;
    position = plane_line_clip(
        vec4(plane_n, 0),
        vec4(plane_d, 0),
        vec4(view_position, 1.0),
        vec4(ray_end, 1.0),
        no_hit);
    return no_hit;
}

