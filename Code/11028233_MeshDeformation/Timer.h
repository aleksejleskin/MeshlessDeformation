/*
	Contains the timer class to calculate FPS and can be used as a game timer
*/
#ifndef TIMER_H
#define TIMER_H

#include <windows.h>

class Timer
{
public:
	Timer();
	~Timer();

	float GetGameTime()const;
	float GetDeltaTime()const;
	bool GetStarted()const;

	void Reset();
	void Start();
	void Stop();
	void Tick();
private:
	double m_secsPerCount, 
		m_deltaTime;

	__int64 m_baseTime,
		m_pausedTime,
		m_stopTime,
		m_prevTime,
		m_currentTime,
		m_startTime;

	bool m_stopped,
		m_started;
};

#endif