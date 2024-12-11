#include <webgpu/webgpu.h>

#define SDL_MAIN_HANDLED
#include <sdl2webgpu.h>
#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__

#include <cassert>
#include <iostream>
#include <vector>
#include <functional>

#include "graphics/gpu.h"
#include "graphics/rendertarget.h"
#include "scheduler.h"
#include "viewport.h"

int main (int, char**) 
{
    Gpu             gpu;
    Scheduler       scheduler;
    WindowTarget    windowTarget(gpu);
    Viewport        viewport(scheduler, gpu, windowTarget);

    scheduler.run();

    return 0;
}