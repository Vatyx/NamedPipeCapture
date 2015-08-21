#pragma once
#ifndef SCOPEGUARD_HXX_DEFINED
#define SCOPEGUARD_HXX_DEFINED
#include <functional>
#include <utility>

template<typename FunctionToExecute>
class Scopeguard
{
   Scopeguard& operator=(const Scopeguard&);
public:
   explicit Scopeguard(FunctionToExecute&& callMe)
      :toBeCalled(std::forward<FunctionToExecute>(callMe))
      , dismissed(false)
   {}
   Scopeguard(Scopeguard&& source)
      :toBeCalled(std::move(source.toBeCalled))
      , dismissed(source.dismissed)
   {
      source.Dismiss();
   }
   inline __forceinline ~Scopeguard()
   {
      if (!dismissed)
         toBeCalled();
   }
   inline __forceinline void Dismiss()
   {
      dismissed = true;
   }

private:
   FunctionToExecute toBeCalled;
   bool dismissed;
};

template<typename FuncToCall>
Scopeguard<FuncToCall> ScopeGuard(FuncToCall&& f)
{
   return Scopeguard<FuncToCall>(std::forward<FuncToCall>(f));
}

#endif