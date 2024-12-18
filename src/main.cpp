#include <webgpu/webgpu.h>

#define SDL_MAIN_HANDLED
#include <sdl2webgpu.h>
#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 

#include "graphics/gpu.h"
#include "graphics/rendertarget.h"
#include "scheduler.h"
#include "viewport.h"
#include "object.h"
#include "animator.h"
#include "io/loader.h"

using namespace glm;

int main (int, char**) 
{
    #ifdef GLM_FORCE_LEFT_HANDED
        std::cout << "GLM left handed" << std::endl;
    #else
        std::cout << "GLM right handed" << std::endl;
    #endif

    #ifdef GLM_FORCE_DEPTH_ZERO_TO_ONE
        std::cout << "Depth 0 .. 1" << std::endl;
    #else
        std::cout << "Depth 1 .. -1" << std::endl;
    #endif

    Gpu             gpu;
    Scheduler       scheduler;
    WindowTarget    windowTarget(gpu);
    Viewport        viewport(scheduler, gpu, windowTarget);

    Object          root;
    CameraObject    camera  = CameraObject(root);
    Object          light   = Object(camera);

    Animator animator(scheduler, [&](double t)
    {
        camera.lookAt(vec3(0, 0, -5), vec3(0,0,0), vec3(0, 1, 0));
        mat4 rotation = rotate(mat4(1.0), radians(float(t * 200.0f)), vec3(0.0f, 1.0f, 0.0f)) * camera.getTransform();
             rotation = rotate(mat4(1.0), radians(float(t * 140.0f)), vec3(1.0f, 0.0f, 0.0f)) * rotation;
        camera.setTransform(rotation);
    });
    animator.start();

    camera.setPerspective(radians(45.0f), 0.1f, 1000.0f);
    camera.lookAt(vec3(0, 0, -5), vec3(0,0,0), vec3(0, 1, 0));
    light.lookAt(vec3(0.5, 0.6, 1), vec3(0, 0, 0), vec3(0, 1, 0));

    viewport.attachCamera(camera);
    viewport.attachLight(light);
    
    std::unique_ptr<Scene> scene = loadModelObjects("./models/DamagedHelmet.glb", root);
    for (auto& renderable : scene->all())
    {
        viewport.attachRenderable(renderable->getRenderable());
    }


    // Object knotObj(root);
    // knotObj.setTransform(translate(mat4(1.0), vec3(1.0,0.0,0.0)));
    // Renderable renderable(knotObj);
    // renderable.mesh.knot(0.5, 0.2, 180, 180);
    // viewport.attachRenderable(renderable);

    scheduler.run();

    return 0;
}