#include "scheduler.h"

#define SDL_MAIN_HANDLED
#include <sdl2webgpu.h>
#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__

#include "viewport.h"
#include "animator.h"

Scheduler::Scheduler()
{
}

Scheduler::~Scheduler()
{
    SDL_Quit();
}

bool shouldClose = false;

void Scheduler::tick()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            shouldClose = true;
            #ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
            #endif
            break;

        default:
            break;
        }
}

    for (Animator* animator: animators)
        animator->animate();

    auto time   = std::chrono::steady_clock::now() - startTime;
    auto milli  = std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
    for (Viewport* viewport: viewports)
        viewport->render();

    nrFrames++;

    if (milli >= 2000 )
    {
        float fps = (float) nrFrames / (milli / 1000.0f);
        std::cout << fps << "fps" << std::endl;
        nrFrames = 0;
        startTime = std::chrono::steady_clock::now();
    }
}

void Scheduler::run()
{
    startTime = std::chrono::steady_clock::now();

    #ifdef __EMSCRIPTEN__
    void *arg = this;
    emscripten_set_main_loop_arg(mainLoop, arg, 0, true);
    #else

    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Could not initialize SDL! Error: " << SDL_GetError() << std::endl;
        return;
    }
    while (!shouldClose)
        tick();

    #endif // __EMSCRIPTEN__
}
