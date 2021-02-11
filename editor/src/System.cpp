#include "System.hpp"

namespace SDLSystem
{
    System* system;
    std::function<void()> render_func;

    void render_update_func()
    {
        render_func();
    }

    System::System(
        std::function<void()> update)
    {
        render_update = update;

        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        {
            cout <<
                "Could not initialize SDL" <<
                endl;
            cout << SDL_GetError() << std::endl;
            SDL_Quit();
            throw false;
        }

        int flags = IMG_INIT_PNG;
        int initted = IMG_Init(flags);
        if ((initted&flags) != flags)
        {
            cout <<
                "Error: SDL_image Error: " <<
                IMG_GetError() <<
                endl;
            SDL_Quit();
            throw false;
        }

        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            cout <<
                "Error while initializing the SDL : " <<
                SDL_GetError() <<
                endl;
            SDL_Quit();
            throw false;
        }

        SDL_DisplayMode DM;
        SDL_GetCurrentDisplayMode(0, &DM);
        display_width = DM.w;
        display_height = DM.h;

        window_width = 1280;//display_width;
        window_height = 720;//display_height;

        InitWindow();
    }

    System::~System()
    {
    }

    void System::SetMouseActive(bool status)
    {
        mouse_delta_x = 0;
        mouse_delta_y = 0;
        mouse_active = status;
        SDL_SetRelativeMouseMode(static_cast<SDL_bool>(status));
    }

    bool System::IsKeyDown(uint16_t key)
    {
        if (key_state.find(key) != key_state.end())
        {
            return key_state[key];
        }
        return false;
    }

    void System::InitWindow()
    {
        render_func = render_update;
        system = this;

        window = SDL_CreateWindow(
            "SDLProject",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            window_width,
            window_height,
            SDL_WINDOW_SHOWN);

        renderer = SDL_CreateRenderer(window, -1, 0);

        const char *error = SDL_GetError();
        if (*error != '\0')
        {
            stringstream err;
            cerr <<
                "SDL Error: " <<
                error <<
                endl;
            SDL_ClearError();
            //throw err.str();
        }
    }

    void System::Destroy()
    {
        SDL_DestroyWindow(window);
    }

    void System::FrameUpdate()
    {
        system->mouse_delta_x = 0;
        system->mouse_delta_y = 0;

        SDL_RenderPresent(renderer);
    }

    bool poll_events()
    {
        system->mouse_down_prev = system->mouse_down;

        SDL_Event event;
        uint16_t key;

        SDL_PumpEvents();

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                return true;
                break;

            case SDL_APP_DIDENTERFOREGROUND:
                SDL_Log("SDL_APP_DIDENTERFOREGROUND");
                break;

            case SDL_APP_DIDENTERBACKGROUND:
                SDL_Log("SDL_APP_DIDENTERBACKGROUND");
                break;

            case SDL_APP_LOWMEMORY:
                SDL_Log("SDL_APP_LOWMEMORY");
                break;

            case SDL_APP_TERMINATING:
                SDL_Log("SDL_APP_TERMINATING");
                break;

            case SDL_APP_WILLENTERBACKGROUND:
                SDL_Log("SDL_APP_WILLENTERBACKGROUND");
                break;

            case SDL_APP_WILLENTERFOREGROUND:
                SDL_Log("SDL_APP_WILLENTERFOREGROUND");
                break;

            case SDL_MOUSEMOTION:
                system->mouse_x = event.motion.x;
                system->mouse_y = event.motion.y;
                system->mouse_delta_x = event.motion.xrel;
                system->mouse_delta_y = event.motion.yrel;
                break;

            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        system->window_width = event.window.data1;
                        system->window_height = event.window.data2;

                        SDL_Log("Window %d resized to %dx%d",
                            event.window.windowID,
                            event.window.data1,
                            event.window.data2);
                        break;
                    }
                }

            case SDL_MOUSEBUTTONUP:
                //if (event.button.button == SDL_BUTTON_LEFT)
                //{
                //    if (!system->mouse_active && event.button.clicks == 2)
                //    {
                //        system->SetMouseActive(true);
                //    }
                //}
                system->mouse_down = false;
                break;

            case SDL_MOUSEBUTTONDOWN:
                system->mouse_down = true;
                break;

            case SDL_KEYDOWN:
                key = static_cast<uint16_t>(event.key.keysym.sym);
                system->key_state[key] = true;
                break;

            case SDL_KEYUP:
                key = static_cast<uint16_t>(event.key.keysym.sym);
                system->key_state[key] = false;
                if (key == 27)
                {
                    system->SetMouseActive(false);
                }
                break;
            }
        }

        system->mouse_click_up = !system->mouse_down && system->mouse_down_prev;
        system->mouse_click_down = system->mouse_down && !system->mouse_down_prev;

        return false;
    }

    void System::Run()
    {
        //SetMouseActive(true);

        bool done = false;

        while (!done)
        {
            done = poll_events();
            render_update_func();
        }

        Destroy();

        SDL_Quit();
    }
}
