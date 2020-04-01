//
//  MLTimer.cpp
//  madronalib
//
//  Created by Randy Jones on 9/10/2018
//

#include <functional>

#include "MLTimer.h"
#include "MLPlatform.h"

// Timers

const int ml::Timers::kMillisecondsResolution = 16;
  
#if ML_MAC

#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach_time.h>

void macTimersCallback (CFRunLoopTimerRef, void *info)
{
  ml::Timers* pTimers = static_cast<ml::Timers*>(info);
  pTimers->tick();
}

void ml::Timers::start(bool runInMainThread)
{
  _inMainThread = runInMainThread;
  if(!_running)
  {
    if(_inMainThread)
    {
      CFRunLoopTimerContext timerContext = {};
      timerContext.info = this;
      float intervalInSeconds = ml::Timers::kMillisecondsResolution*0.001f;
      pTimersRef = CFRunLoopTimerCreate (kCFAllocatorDefault, CFAbsoluteTimeGetCurrent() + intervalInSeconds, intervalInSeconds, 0, 0, macTimersCallback, &timerContext);
      if (pTimersRef)
      {
        _running = true;
        CFRunLoopTimerRef pTimerRef = static_cast<CFRunLoopTimerRef>(pTimersRef);
        CFRunLoopAddTimer (CFRunLoopGetMain(), pTimerRef, kCFRunLoopCommonModes);
      }
    }
    else
    {
     _running = true;
      runThread = std::thread { [&](){ run(); } };
    }
  }
}

void ml::Timers::stop()
{
  if(_inMainThread)
  {
      if( pTimersRef)
      {
        CFRunLoopTimerRef pLoopRef = static_cast<CFRunLoopTimerRef>(pTimersRef);
        CFRunLoopRemoveTimer (CFRunLoopGetMain(), pLoopRef, kCFRunLoopCommonModes);
        pTimersRef = nullptr;
      }
  }
  else
  {
    // signal thread to exit
    _running = false;
    runThread.join();
  }
}

#elif ML_WINDOWS

#include <windows.h>

constexpr int kWinTimerId = 1;

void CALLBACK winTimersCallback (HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR idEvent, DWORD /*dwTime*/)
{
  ml::Timers* pTimers = static_cast<ml::Timers*>(ml::Timers::pTimersRef);
  if(pTimers)
  {
    pTimers->tick();
  }
}

void ml::Timers::start(bool runInMainThread)
{
  _inMainThread = runInMainThread;
  if(!_running)
  {
    if (_inMainThread)
    {
      auto id = SetTimer(0, kWinTimerId, ml::Timers::kMillisecondsResolution, winTimersCallback);
      if (id)
      {
        _running = true;
      }
    }
    else
    {
      _running = true;
      runThread = std::thread { [&](){ run(); } };
    }
  }
}

void ml::Timers::stop()
{ 
  if(_inMainThread)
  {
    KillTimer(0, kWinTimerId);
  }
  else
  {
    // signal thread to exit
    _running = false;
    runThread.join();
  }   
}

#elif ML_LINUX

void Timers::start(bool runInMainThread)
{
  if(!_running)
  {
    _running = true;
    runThread = std::thread { [&](){ run(); } };
  }
}

#endif

void ml::Timers::tick(void)
{
  time_point<system_clock> now = system_clock::now();
  std::unique_lock<std::mutex> lock(mSetMutex);

  for(auto t : timerPtrs)
  {
    if(t->mCounter != 0)
    {
      if(now - t->mPreviousCall > t->mPeriod)
      {
        t->myFunc();
        if(t->mCounter > 0)
        {
          t->mCounter--;
        }
        t->mPreviousCall = now;
      }
    }
  }
}

void ml::Timers::run(void)
{
  while(_running)
  {
    std::this_thread::sleep_for(milliseconds(Timers::kMillisecondsResolution));
    tick();
  }
}

// Timer

ml::Timer::Timer() noexcept
{
  std::unique_lock<std::mutex> lock(_timers->mSetMutex);
  _timers->insert(this);
}

ml::Timer::~Timer()
{
  std::unique_lock<std::mutex> lock(_timers->mSetMutex);
  _timers->erase(this);
}

void ml::Timer::stop()
{
  // More lightweight ways of handling a race on mCounter are possible, but as
  // stopping a timer is an infrequent operation we use the mutex for brevity.
  std::unique_lock<std::mutex> lock(_timers->mSetMutex);
  mCounter = 0;
}

