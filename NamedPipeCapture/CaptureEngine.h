#pragma once

#include <memory>

class NamedPipe;
struct globalStruct;

void StartNamedPipeServer(globalStruct& obj);
void InitCycle(std::shared_ptr<NamedPipe>& pipe);

void CleanUpEverything();