#pragma once
#include "../DiscordSDK/include/discord_rpc.h"
#include "../DiscordSDK/include/discord_register.h"
#include <Windows.h>
#include <string>

class Discord {
public:
	void Initialize(const char* appId);
	void Update(const char* detail, const char* state, const char* largeImageText, const char* largeImageKey, const char* smallImageText, const char* smallImageKey, const char* button1Label, const char* button1Url, const char* button2Label, const char* button2Url, int64_t startTimestamp);
};