#include "Timer.h"

Timer::Timer() : m_secsPerCount(0.0), m_deltaTime(0.0),
	m_baseTime(0), m_pausedTime(0), m_stopTime(0), 
	m_prevTime(0), m_currentTime(0), m_startTime(0), m_stopped(false),
	m_started(false)
{
	//Find out the original count between each second
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_secsPerCount = 1.0 / (double)countsPerSec;
}

Timer::~Timer()
{

}

float Timer::GetGameTime() const
{
	float result;

	if( m_stopped )
	{
		//return the correct game time if the timer has been paused
		return (float)(((m_stopTime - m_pausedTime)-m_baseTime)*m_secsPerCount);
	}
	else
	{
		 //return the correct game time if the timer is still going
		return (float)(((m_currentTime-m_pausedTime)-m_baseTime)*m_secsPerCount);
	}
}

float Timer::GetDeltaTime() const
{
	return (float)m_deltaTime;
}

void Timer::Reset()
{
	__int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	m_baseTime = currentTime;
	m_prevTime = currentTime;
	m_stopTime = 0;
	m_stopped = false;
}

void Timer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*) &startTime);

	m_startTime = startTime;

	if(m_stopped)
	{
		m_pausedTime += (startTime - m_stopTime);
		m_prevTime = startTime;
		m_stopTime = 0;
		m_stopped = false;
	}

	m_started = true;
}

void Timer::Stop()
{
	if(!m_stopped)
	{
		__int64 currentTime;
		QueryPerformanceCounter((LARGE_INTEGER*) &currentTime);

		m_stopTime = currentTime;
		m_stopped = true;
		m_started = false;
	}
}

void Timer::Tick()
{
	if(m_stopped)
	{
		m_deltaTime = 0.0;
		return;
	}

	__int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*) &currentTime);

	m_currentTime = currentTime - m_startTime;

	m_deltaTime = (m_currentTime - m_prevTime) * m_secsPerCount;

	m_prevTime = m_currentTime;

	if(m_deltaTime < 0.0)
	{
		m_deltaTime = 0.0;
	}
}

bool Timer::GetStarted() const
{
	return m_started;
}