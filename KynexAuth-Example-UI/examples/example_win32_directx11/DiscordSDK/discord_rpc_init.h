#pragma once

#include "Includes\discord_register.h"
#include "Includes\discord_rpc.h"
#include <Windows.h>

class Discord {
public:
    void Initialize();
	void Update(const char* detail, const char* state, const char* largeImageText);
};