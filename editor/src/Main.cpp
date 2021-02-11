#include "Main.hpp"

#include "Math.hpp"
#include "Drawing.hpp"
#include "Spline.hpp"
#include "System.hpp"

using namespace SDLSystem;

enum class ApplicationState
{
    DEFAULT,
    VIEW,
    PLACEMENT,
    MOVEMENT
};

enum class PickingType
{
    NONE,
    POINT,
    CONTROL,
    NORMAL,
};

ApplicationState app_state = ApplicationState::DEFAULT;

SystemPtr sys;

float view_yaw = glm::radians(-90.0f);
float view_pitch = glm::radians(0.0f);
float view_speed_slow = 0.005f;
float view_speed_fast = 0.03f;

vec3 view_up_vector = vec3(0, 1, 0);
vec3 view_side_vector = vec3(1, 0, 0);

float point_size = 20.0f;
size_t point_picked_id = 0;
PickingType point_picked_type = PickingType::NONE;
Spline path;

void init()
{
    renderer = sys->renderer;
}

void update()
{
    SDL_GetWindowSize(sys->window, &window_width, &window_height);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (app_state == ApplicationState::VIEW)
    {
        view_yaw += static_cast<float>(sys->mouse_delta_x * 10) / window_width;
        view_pitch += static_cast<float>(sys->mouse_delta_y * 10) / window_height;
    }

    vec3 view_vector(
        cos(view_yaw) * cos(view_pitch),
        sin(view_pitch),
        sin(view_yaw) * cos(view_pitch));

    vec3 view_side_vector = glm::cross(view_up_vector, view_vector);

    float view_speed = sys->IsKeyDown(224) ? view_speed_slow : view_speed_fast;

    if (sys->IsKeyDown(119)) // W
    {
        view_position += view_vector * -view_speed;
    }
    if (sys->IsKeyDown(115)) // S
    {
        view_position += view_vector * view_speed;
    }
    if (sys->IsKeyDown(97))  // A
    {
        view_position += view_side_vector * -view_speed;
    }
    if (sys->IsKeyDown(100)) // D
    {
        view_position += view_side_vector * view_speed;
    }

    float aspect = static_cast<float>(window_width) / window_height;
    projection = glm::perspective(
        glm::radians(view_fov),
        aspect,
        view_near_z,
        100.0f);

    projection[1][1] = -projection[1][1];

    view = glm::lookAt(
        view_position,
        view_position - view_vector,
        view_up_vector);

    projection_view = projection * view;

    switch (app_state)
    {
        case ApplicationState::DEFAULT:
            if (sys->IsKeyDown(101))
            {
                app_state = ApplicationState::PLACEMENT;
                break;
            }

            if (sys->IsKeyDown(32))
            {
                app_state = ApplicationState::VIEW;
                sys->SetMouseActive(true);
                break;
            }

            // control point picking
            {
                if (sys->mouse_click_down)
                {
                    // render control points for picking
                    bool point_pick_active = false;

                    // points
                    uint32_t color_counter = 1;
                    for (auto point : path.points)
                    {
                        SDL_SetRenderDrawColor(renderer,
                            1,
                            (color_counter >> 8)  & 0x0000ff00,
                            (color_counter >> 0)  & 0x000000ff,
                            255);
                        draw_point_3d(point, point_size);
                        color_counter++;
                    }

                    // control points
                    color_counter = 1;
                    for (auto i = 0; i < path.points.size(); i++)
                    {
                        SDL_SetRenderDrawColor(renderer,
                            2,
                            (color_counter >> 8) & 0x0000ff00,
                            (color_counter >> 0) & 0x000000ff,
                            255);

                        vec3 control = path.points[i] + path.controls[i];
                        draw_point_3d(control, point_size);
                        color_counter++;
                    }

                    // normal
                    color_counter = 1;
                    for (auto i = 0; i < path.points.size(); i++)
                    {
                        SDL_SetRenderDrawColor(renderer,
                            3,
                            (color_counter >> 8) & 0x0000ff00,
                            (color_counter >> 0) & 0x000000ff,
                            255);

                        vec3 control = path.points[i] + path.normals[i] * 0.5f;

                        draw_point_3d(
                            control,
                            point_size);
                        color_counter++;
                    }

                    // get color of picked point
                    SDL_Color rgb;
                    const Uint32 format = SDL_PIXELFORMAT_ARGB8888;
                    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(
                        0, window_width, window_height, 32, format);
                    SDL_LockSurface(surface);
                    SDL_RenderReadPixels(
                        sys->renderer, NULL, format, surface->pixels, surface->pitch);
                    auto data = GetPixel(surface, sys->mouse_x, sys->mouse_y);
                    SDL_GetRGB(data, surface->format, &rgb.r, &rgb.g, &rgb.b);

                    uint32_t id = (rgb.g << 8) + rgb.b;
                    uint32_t type = rgb.r;

                    if (id > 0)
                    {
                        id = id - 1;
                        point_pick_active = true;
                        point_picked_id = id;
                        point_picked_type = static_cast<PickingType>(type);
                    }
                    SDL_UnlockSurface(surface);
                    SDL_FreeSurface(surface);

                    if (point_pick_active)
                    {
                        app_state = ApplicationState::MOVEMENT;
                        break;
                    }
                }
            }
            break;

        case ApplicationState::VIEW:
            if (!sys->mouse_active)
            {
                app_state = ApplicationState::DEFAULT;
                break;
            }
            break;

        case ApplicationState::MOVEMENT:
            if (!sys->mouse_down)
            {
                app_state = ApplicationState::DEFAULT;
                break;
            }

            // move control point
            {
                vec3 point_position =
                    path.points[point_picked_id];

                vec3 control_position =
                    path.controls[point_picked_id] +
                    point_position;

                vec3 plane_position = point_position;
                vec3 plane_normal = view_up_vector;

                if (point_picked_type == PickingType::POINT)
                {
                    plane_position = point_position;
                }
                else if (point_picked_type == PickingType::CONTROL)
                {
                    plane_position = control_position;
                }
                else if (point_picked_type == PickingType::NORMAL)
                {
                    plane_position = point_position;
                    plane_normal = glm::normalize(path.controls[point_picked_id]);

                    if (glm::dot(view_position - plane_position, plane_normal) < 0)
                    {
                        plane_normal = -plane_normal;
                    }
                }

                if (point_picked_type != PickingType::NORMAL)
                {
                    if (sys->IsKeyDown(102))
                    {
                        vec3 n = view_position - plane_position;
                        n.y = 0;
                        plane_normal = glm::normalize(n);
                    }
                }

                vec3 ray_intersection_pos;
                bool no_hit = picking_raycast(
                    ray_intersection_pos,
                    plane_normal,
                    plane_position,
                    sys->mouse_x,
                    sys->mouse_y);

                if (!no_hit)
                {
                    if (point_picked_type == PickingType::POINT)
                    {
                        path.MovePoint(
                            point_picked_id,
                            ray_intersection_pos);
                    }
                    else if(point_picked_type == PickingType::CONTROL)
                    {
                        path.MoveControl(
                            point_picked_id,
                            ray_intersection_pos);
                    }
                    else if (point_picked_type == PickingType::NORMAL)
                    {
                        path.MoveNormal(
                            point_picked_id,
                            ray_intersection_pos);
                    }
                }
            }
            break;

        case ApplicationState::PLACEMENT:
            if (!sys->IsKeyDown(101))
            {
                app_state = ApplicationState::DEFAULT;
                break;
            }

            // place new control point
            {
                vec3 ray_intersection_pos;
                bool no_hit = picking_raycast(
                    ray_intersection_pos,
                    vec3(0, 1, 0),
                    vec3(0, 0, 0),
                    sys->mouse_x,
                    sys->mouse_y);

                if (!no_hit)
                {
                    vec4 p = projection_view * vec4(ray_intersection_pos, 1.0f);
                    p = project_screen(p);
                    int rad = static_cast<int>(point_size / p.w);
                    draw_circle(
                        static_cast<Sint16>(p.x),
                        static_cast<Sint16>(p.y),
                        rad,
                        255, 0, 0);
                }

                if (sys->mouse_click_up)
                {
                    if (!no_hit)
                    {
                        path.InsertPoint(ray_intersection_pos);
                    }
                    app_state = ApplicationState::DEFAULT;
                    break;
                }
            }
            break;
        default:
            break;
    }

    // rendering
    {
        float grid_scale = 1;

        // render grid

        SDL_SetRenderDrawColor(renderer, 128, 128, 128, SDL_ALPHA_OPAQUE);

        for (int16_t i = -20; i < 21; i++)
        {
            vec3 x0 = vec3(i, 0, 20) * grid_scale;
            vec3 x1 = vec3(i, 0, -20) * grid_scale;
            draw_line_3d(x0, x1);

            vec3 z0 = vec3(20, 0, i) * grid_scale;
            vec3 z1 = vec3(-20, 0, i) * grid_scale;
            draw_line_3d(z0, z1);
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

        // render track
        if (path.points.size() > 2)
        {
            float path_width = 0.2f;

            float offset = 0;
            float offset_prev = 0;

            vec3 track_left_start;
            vec3 track_right_start;

            vec3 track_left;
            vec3 track_right;

            vec3 track_left_prev;
            vec3 track_right_prev;

            bool is_start_point_set = false;

            for (float t = 0; t < path.total_length; t += 0.5f)
            {
                offset_prev = offset;
                track_left_prev = track_left;
                track_right_prev = track_right;

                offset = path.GetNormalisedOffset(t);

                vec3 position = path.GetPoint(offset);
                vec3 gradient = glm::normalize(path.GetGradient(offset));
                vec3 side = glm::cross(gradient, path.GetNormal(offset)) * path_width;

                track_left = position + side;
                track_right = position - side;

                vec3 track_left_ground = vec3(track_left.x, 0, track_left.z);
                vec3 track_right_ground = vec3(track_right.x, 0, track_right.z);
                vec3 track_left_ground_prev = vec3(track_left_prev.x, 0, track_left_prev.z);
                vec3 track_right_ground_prev = vec3(track_right_prev.x, 0, track_right_prev.z);

                SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

                draw_line_3d(track_left, track_right);
                draw_line_3d(track_left, track_left_ground);
                draw_line_3d(track_right, track_right_ground);

                if (is_start_point_set)
                {
                    draw_line_3d(track_left_prev, track_left);
                    draw_line_3d(track_right_prev, track_right);
                }
                else
                {
                    track_left_start = track_left;
                    track_right_start = track_right;
                    is_start_point_set = true;
                }
            }

            draw_line_3d(track_left, track_left_start);
            draw_line_3d(track_right, track_right_start);
        }

        // render track control points
        {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

            for (auto point : path.points)
            {
                draw_point_3d(point, point_size);
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

            for (auto i = 0; i < path.points.size(); i++)
            {
                vec3 point = path.points[i];
                vec3 direction = path.controls[i];
                vec3 control = point + direction;

                draw_point_3d(
                    control,
                    point_size);

                draw_line_3d(
                    point + direction,
                    point - direction);
            }

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

            for (auto i = 0; i < path.points.size(); i++)
            {
                vec3 point = path.points[i];
                vec3 direction = path.normals[i];
                vec3 control = point + direction * 0.5f;

                draw_point_3d(
                    control,
                    point_size);

                draw_line_3d(
                    point,
                    control);
            }
        }
    }

    sys->FrameUpdate();
}

int main(int argc, char *argv[])
{
    sys = make_shared<System>([=]()
    {
        update();
    });

    init();

    sys->Run();

    return 0;
}
