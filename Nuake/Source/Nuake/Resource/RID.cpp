#include "RID.h"

using namespace Nuake;

std::vector<RID*> RID::Handles;
std::queue<RID::RemapRequest> RID::RemapQueue;
std::mutex RID::RemapMutex;