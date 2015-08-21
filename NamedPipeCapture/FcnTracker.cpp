#include "FcnTracker.hxx"

FunctionUsageTracker::FunctionUsageTracker()
   : m_ThreadsInUse()
   , m_SignalCheck(false)
   , m_Signal()
   , m_Lock()
{
}

FunctionUsageTracker::~FunctionUsageTracker()
{

}

void FunctionUsageTracker::AddThreadBlocking(const std::thread::id& id)
{
   std::lock_guard<std::mutex> lg(m_Lock);
   m_ThreadsInUse.push_back(id);
}

void FunctionUsageTracker::RemoveThreadBlocking(const std::thread::id& id)
{
   std::lock_guard<std::mutex> lg(m_Lock);
   auto foundItem = std::find(std::begin(m_ThreadsInUse), std::end(m_ThreadsInUse), id);
   if (foundItem != std::end(m_ThreadsInUse))
   {
      m_ThreadsInUse.erase(foundItem);
   }
   if (m_SignalCheck && m_ThreadsInUse.empty())
   {
      m_SignalCheck = false;
      m_Signal.notify_all();
   }
}

