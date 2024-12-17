#pragma once

#include "scheduler.h"
#include <chrono>
#include <functional>

// The animator calls animatecallback once for every frame. 
class Animator
{
public:
    Animator(Scheduler& scheduler, std::function<void(double)> animateCallback);
    ~Animator();

    void animate();

    void start();
    void stop();

private:
    bool isRunning = false;

    Scheduler&                  scheduler;
    std::function<void(double)> animateCallback;

    std::chrono::time_point<std::chrono::steady_clock> startTime;

    Animator (const Animator&)              = delete;
    Animator& operator= (const Animator&)   = delete;

};