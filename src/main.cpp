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
#include "inputs/roll.h"

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

    Object          root;
    Object          sceneParent(root);
    CameraObject    camera  = CameraObject(root);
    LightObject     light   = LightObject(camera);

    Cubemap reflectionMap;
    reflectionMap.positiveX = loadImage("./cubemap/px.png");
    reflectionMap.negativeX = loadImage("./cubemap/nx.png");
    reflectionMap.positiveY = loadImage("./cubemap/py.png");
    reflectionMap.negativeY = loadImage("./cubemap/ny.png");
    reflectionMap.positiveZ = loadImage("./cubemap/pz.png");
    reflectionMap.negativeZ = loadImage("./cubemap/nz.png"); 

    std::unique_ptr<Scene> scene = loadModelObjects("./models/DamagedHelmet.glb", sceneParent);
    scene->m_environmentMap = &reflectionMap;

    std::unique_ptr<Viewport>        viewport = std::make_unique<Viewport>(scheduler, gpu, windowTarget, *scene);

    sceneParent  .setTransform(scale(mat4(1.0), vec3(5.0)));

    Box box = scene->getBox();
    vec3 sceneCenter = Space::pos(box.center(), Space(), camera.getParentSpace());
    float diameter   = Space::dir(box.max - box.min, Space(), camera.getParentSpace()).length();

    vec3 centerInCameraSpace = Space::pos(box.center(), Space(), camera.getSpace());

    camera.setPerspective(radians(45.0f), 0.1f, 1000.0f);
    light.lookAt(vec3(1.0, 0.6, 0), vec3(0, 0, 1), vec3(0, 1, 0));

    viewport->attachCamera(camera);
    viewport->attachLight(light);

    for (auto& renderable : scene->all())
        viewport->attachRenderable(renderable->getRenderable());

    camera.lookAt(sceneCenter + vec3(0, 0, diameter * 5), sceneCenter, vec3(0, 1, 0));
    //camera.lookAt(vec3(0, 0, -5), vec3(0,0,0), vec3(0, 1, 0));
    RollInput   cameraInput     = RollInput(*viewport, camera);

    scheduler.run();

    return 0;
}