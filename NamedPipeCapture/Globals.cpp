#include "Globals.h"

#include <atomic>
#include "NamedPipe.h"
#include "NamedPipeServer.h"

std::atomic<globalStruct*> globalStruct::globalObject = nullptr;

globalStruct* globalStruct::GetGlobals()
{
   return globalStruct::globalObject.load();
}

bool globalStruct::InitGlobals(globalStruct* obj)
{
   globalStruct* pVal = nullptr;
   return globalObject.compare_exchange_strong(pVal, obj);
}

globalStruct::~globalStruct() {}
