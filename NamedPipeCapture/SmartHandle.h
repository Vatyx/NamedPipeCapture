#pragma once
#ifndef DEFINED_SMARTHANDLE_H
#define DEFINED_SMARTHANDLE_H

#include <windows.h>
#include "boost/noncopyable.hpp"

template<typename Deleter, HANDLE invalidVal>
class SmartHandle : private ::boost::noncopyable
{
private:
   HANDLE m_handle;
public:
   explicit SmartHandle(HANDLE h_in = invalidVal) :m_handle(h_in) {};
   ~SmartHandle()
   {
      Destroy();
   }
   SmartHandle(SmartHandle&& other)
      : m_handle(other.m_handle)
   {
      other.m_handle = invalidVal;
   }
   SmartHandle& operator=(SmartHandle&& other)
   {
      if (&other != this)
      {
         Destroy();
         m_handle = other.m_handle;
         other.m_handle = invalidVal;
      }
      return *this;
   }

   SmartHandle& operator=(HANDLE h_in)
   {
      if (m_handle != h_in)
      {
         Destroy();
         m_handle = h_in;
      }
      return *this;
   }
   void Reset(HANDLE h_in = invalidVal)
   {
      Destroy();
      m_handle = h_in;
   }
   explicit operator HANDLE() const
   {
      return m_handle;
   }
   bool IsValid() const { return m_handle != invalidVal; }

   HANDLE ReleaseHandle()
   {
      auto retVal = m_handle;
      m_handle = invalidVal;
      return retVal;
   }

private:
   void Destroy()
   {
      if (m_handle != invalidVal)
      {
         Deleter()(m_handle);
         m_handle = invalidVal;
      }
   }
};

struct SmartHandleCloseHandle
{
   void operator()(HANDLE hndl)
   {
      ::CloseHandle(hndl);
   }
};
struct FindFileCloseHandle
{
   void operator()(HANDLE hndl)
   {
      FindClose(hndl);
   }
};
typedef SmartHandle<SmartHandleCloseHandle, NULL> EventHandle;
typedef SmartHandle<SmartHandleCloseHandle, INVALID_HANDLE_VALUE> NamedPipeHandle;
typedef SmartHandle<SmartHandleCloseHandle, INVALID_HANDLE_VALUE> FileHandle;
typedef SmartHandle<FindFileCloseHandle, INVALID_HANDLE_VALUE> FindFileHandle;
#endif