#include "SDL3/SDL.h"
#include "Timer.h"
#include <algorithm>

Timer::Timer():targetFrameTime (0), fps(0),prevFrame(0), currFrame(0),frequency(0),dt(0){}
Timer::~Timer() {}
void Timer::Initialize(unsigned int fps_)
{
	fps = fps_;
	frequency = SDL_GetPerformanceFrequency();
	targetFrameTime = frequency / static_cast<uint64_t>(fps);
	prevFrame = SDL_GetPerformanceCounter();

}

void Timer::StartFrameTime()
{
	currFrame = SDL_GetPerformanceCounter();

	dt = static_cast<float>(currFrame - prevFrame) / static_cast<float>(frequency);
	// Capped dt should change the max to a variable later
	// Calculate dt and cache it for this frame
	dt = SDL_clamp(dt, 0.0f, 0.03f);
	prevFrame = currFrame;
}

void Timer::EndFrameTime()
{

	uint64_t frameTime = SDL_GetPerformanceCounter() - currFrame;

	uint64_t sleepTime = std::max<int64_t>(0, targetFrameTime - frameTime);

	//conversion form ticks to milliseconds
	sleepTime = (sleepTime * 1000) / frequency; 

	if( sleepTime > 0 )
		SDL_Delay(static_cast<uint32_t>(sleepTime));

}
