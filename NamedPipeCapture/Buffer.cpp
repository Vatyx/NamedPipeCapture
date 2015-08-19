#include "buffer.h"
#include <utility>
#include <vector>
#include <memory>

namespace StreamerTools
{
Buffer::Buffer()
   : m_Data(nullptr)
   , m_Size(0)
   , m_TimePoint(time_point::min())
   , m_Action(Action::INVALID)
   {
   }

   Buffer::Buffer(std::unique_ptr<char []> ptr, size_t theSize) : Buffer()
   {
      m_Data = std::move(ptr);
      m_Size = theSize;
   }

Buffer::Buffer(std::unique_ptr<char[]> ptr, size_t s, time_point d, Action act)
   : Buffer()
   {
      m_Data = std::move(ptr);
      m_Size = s;
      m_TimePoint= d;
      m_Action = act;
   }

   Buffer::Buffer(Buffer&& b)
      : m_Data(std::move(b.m_Data))
      , m_Size(std::move(b.m_Size))
      , m_TimePoint(std::move(b.m_TimePoint))
      , m_Action(std::move(b.m_Action))
   {
   }

   Buffer& Buffer::operator=(Buffer&& b)
   {
      m_Data = std::move(b.m_Data);
      m_Size = std::move(b.m_Size);
	  m_TimePoint = std::move(b.m_TimePoint);
      m_Action = std::move(b.m_Action);
      return *this;
   }

std::vector<std::pair<std::unique_ptr<char[]>, std::size_t>>
      Buffer::split(size_t segsize)
   {
   std::vector<std::pair<std::unique_ptr<char[]>, std::size_t>> retval;

      auto curIter = m_Data.get();
      auto endIter = m_Data.get() + m_Size;

      while (curIter < endIter)
      {
         auto distance = endIter - curIter;
         if (distance > segsize)
         {
            distance = segsize;
         }
      auto NewData = std::unique_ptr<char[]>(new char[distance]);
         memcpy_s(NewData.get(), distance, curIter, distance);
         retval.emplace_back(std::move(NewData), distance);
         curIter += distance;
      }

	  return retval;
   }

std::pair<std::unique_ptr<char[]>, size_t> Buffer::getMemory()
   {
      auto retval = std::make_pair(std::move(m_Data), m_Size);
      m_Size = 0;
      return retval;
   }
}