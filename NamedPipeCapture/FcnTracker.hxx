#pragma once
#ifndef FCNTRACKER_HXX_DEFINED
#define FCNTRACKER_HXX_DEFINED

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>

class FunctionUsageTracker
{
public:
   FunctionUsageTracker();
   FunctionUsageTracker(const FunctionUsageTracker&) = delete;
   FunctionUsageTracker(FunctionUsageTracker&&) = delete;
   ~FunctionUsageTracker();
   void AddThreadBlocking(const std::thread::id& id);
   void RemoveThreadBlocking(const std::thread::id& id);
   template<typename Rep, typename PeriodType>
   void WaitUntilThreadVecEmpty(const std::chrono::duration<Rep, PeriodType>& duration)
   {
      std::unique_lock<std::mutex> ul(m_Lock);
      if (m_ThreadsInUse.empty())
         return;
      m_SignalCheck = true;
      m_Signal.wait_for(ul, duration, [this]
      {
         return !m_SignalCheck;
      });
   }
private:
   std::vector<std::thread::id> m_ThreadsInUse;
   std::atomic<bool> m_SignalCheck;
   std::condition_variable m_Signal;
   mutable std::mutex m_Lock;
};

#endif