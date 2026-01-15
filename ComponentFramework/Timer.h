#ifndef TIMER_H
#define TIMER_H

class Timer {
public:
	Timer();
	~Timer();

	Timer(const Timer&) = delete;
	Timer(Timer&&) = delete;
	Timer& operator=(const Timer&) = delete;
	Timer& operator=(Timer&&) = delete;

	/// <summary>
	/// Initialize the timer with the target frames per second
	/// </summary>
	/// <param name="fps_"></param>
	void Initialize(unsigned int fps_);	
	/// <summary>
	/// Returns the time elapsed since the last update/frame.
	/// </summary>
	/// <returns>The elapsed time (delta time) as a float, in seconds.</returns>
	float getDeltaTime() { return dt; }
	/// <summary>
	/// Call at the start of each frame to begin timing
	/// </summary>
	void StartFrameTime();
	/// <summary>
	/// Call at the end of each frame to cap the frame rate
	/// if necessary it sleeps the remaining time to hit the target frame time
	/// </summary>
	void EndFrameTime();	
	
private:
		
	uint64_t targetFrameTime;
	unsigned int fps;
	uint64_t prevFrame;
	uint64_t currFrame;
	uint64_t frequency;
	float dt;

	
};

#endif
