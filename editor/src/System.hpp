#pragma once

#if defined (_WIN32) || defined (__WIN32__)
#define SYSTEM_PLATFORM_WIN
#elif defined (linux) || defined (__linux)
#define SYSTEM_PLATFORM_LINUX
#define SYSTEM_PLATFORM_POSIX
#elif defined (__APPLE__) || defined (MACOSX) || defined (macintosh) || defined (Macintosh)
#define SYSTEM_PLATFORM_DARWIN
#define SYSTEM_PLATFORM_POSIX
#endif

#if defined (__i386__) || defined (_M_IX86)
#define SYSTEM_PLATFORM_ARCH_X86
#elif defined (__x86_64__) || defined (_M_AMD64)
#define SYSTEM_PLATFORM_ARCH_X86_64
#endif

#if defined (_MSC_VER)
#define SYSTEM_COMPILER_MSVCPP
#define SYSTEM_COMPILER_FEATURE_NULLPTR
#else
#define SYSTEM_COMPILER_CLANG
#define SYSTEM_COMPILER_FEATURE_NULLPTR
#endif

#if defined (SYSTEM_COMPILER_MSVCPP)
#define __func__ __FUNCTION__
#else
#define __func__ __PRETTY_FUNCTION__
#endif

#ifdef PATH_MAX
#undef PATH_MAX
#endif

#define PATH_MAX 256

#if defined (SYSTEM_PLATFORM_IOS)
#include <CoreFoundation/CoreFoundation.h>
#elif defined (SYSTEM_PLATFORM_OSX)
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>
#elif defined (SYSTEM_PLATFORM_WIN)
#elif defined (EMSCRIPTEN)
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <memory>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <math.h>
#include <map>
#include <memory>

using std::cout;
using std::cerr;
using std::endl;
using std::stringstream;
using std::shared_ptr;
using std::static_pointer_cast;
using std::make_shared;
using std::shared_ptr;

#if defined (SYSTEM_PLATFORM_DARWIN)
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#else
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_image.h>
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/type_aligned.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/quaternion.hpp>
#include "../../lib/glm/glm/gtc/quaternion.hpp"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat2;
using glm::mat3;
using glm::mat4;
using glm::mat2x2;
using glm::mat3x3;
using glm::mat4x4;
using glm::quat;

using glm::dvec2;
using glm::dvec3;
using glm::dvec4;
using glm::dmat2x2;
using glm::dmat3x3;
using glm::dmat4x4;
using glm::dquat;
using glm::aligned_vec4;
using glm::aligned_vec3;

namespace SDLSystem
{
    class System
    {
    private:
        void InitWindow();

    public:
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;

        System(
            std::function<void()> update);
        virtual ~System();

        void Run();
        void FrameUpdate();
        void Destroy();

        void SetMouseActive(bool status);
        bool IsKeyDown(uint16_t key);

        std::function<void()> render_update;
        std::map<uint16_t, bool> key_state;

        bool mouse_active = false;

        uint16_t display_width = 0;
        uint16_t display_height = 0;
        uint16_t window_width = 0;
        uint16_t window_height = 0;

        int16_t mouse_x = -1;
        int16_t mouse_y = -1;
        int16_t mouse_delta_x = 0;
        int16_t mouse_delta_y = 0;

        bool mouse_down_prev = false;
        bool mouse_down = false;

        bool mouse_click_up = false;
        bool mouse_click_down = false;
    };

    using SystemPtr = shared_ptr<System>;

    static Uint32 GetPixel(SDL_Surface *surf, int x, int y)
    {
        //This function returns pixels color
        int bpp = surf->format->BytesPerPixel;
        Uint8 *p = (Uint8 *)surf->pixels + y * surf->pitch + x * bpp;

        switch (bpp)
        {
        case 1:
            return *p;

        case 2:
            return *(Uint16 *)p;

        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;

        case 4:
            return *(Uint32 *)p;

        default:
            return 0;
        }
    }
}
