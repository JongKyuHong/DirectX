#include "timerclass.h"

TimerClass::TimerClass()
{
}

TimerClass::TimerClass(const TimerClass&)
{
}

TimerClass::~TimerClass()
{
}

bool TimerClass::Initialize()
{
    INT64 frequency;
    
    // 초당 사이클 수를 가져온다.
    QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
    if (frequency == 0)
    {  
        return false;
    }
    
    // 부동 소수점으로 저장
    m_frequency = (float)frequency;
    
    // 초기 시작 시간을 가져온다.
    QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);
    
    return true;
}

void TimerClass::Frame()
{
    INT64 currentTime;
    INT64 elapsedTicks;
    
    QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
    elapsedTicks = currentTime - m_startTime;
    
    m_frameTime = (float)elapsedTicks / m_frequency;
    
    m_startTime = currentTime;
    
    return;
}

float TimerClass::GetTime()
{
    return m_frameTime;
}
