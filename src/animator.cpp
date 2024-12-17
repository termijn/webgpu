#include "animator.h"

Animator::Animator(Scheduler &scheduler_, std::function<void(double)> animateCallback_)
    : scheduler         (scheduler_)
    , animateCallback   (animateCallback_)
{
    scheduler_.animators.emplace_back(this);
}

Animator::~Animator()
{
    auto it = std::find(scheduler.animators.begin(), scheduler.animators.end(), this);
    scheduler.animators.erase(it);
}

void Animator::animate()
{
    if (!isRunning) return;
    
    auto time   = std::chrono::steady_clock::now() - startTime;
    auto milli  = std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
    animateCallback(milli / 1000.0);
}

void Animator::start()
{
    startTime = std::chrono::steady_clock::now();
    isRunning = true;
}

void Animator::stop()
{
    isRunning = false;
}
