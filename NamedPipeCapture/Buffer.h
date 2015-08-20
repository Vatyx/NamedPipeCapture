#pragma once
#ifndef _BUFFER_H_INCLUDED_
#define _BUFFER_H_INCLUDED_
//custom buffer class
#include <chrono>
#include <vector>
#include <memory>

namespace StreamerTools
{

enum class Action : short
{
   INVALID,
   READ,
   WRITE,
   END
};

class Buffer
{
   using time_point = std::chrono::high_resolution_clock::time_point;

public:
   Buffer();
   Buffer(std::unique_ptr<char[]>, size_t);
   Buffer(std::unique_ptr<char[]>, size_t, time_point, Action);

   std::vector<std::pair<std::unique_ptr<char[]>, std::size_t>>
   split(size_t segsize);

   std::pair<std::unique_ptr<char[]>, size_t> getMemory();
   size_t getSize() const { return m_Size; }
   time_point getTime() const { return m_TimePoint; }
   Action getAction() const { return m_Action; }

   Buffer(const Buffer& b) = delete;
   Buffer& operator=(const Buffer& b) = delete;
   Buffer(Buffer&& b);
   Buffer& operator=(Buffer&& b);

private:
   std::unique_ptr<char[]> m_Data;
   size_t m_Size;
   time_point m_TimePoint;
   Action m_Action;
};
}
#endif