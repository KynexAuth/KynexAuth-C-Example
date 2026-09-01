#include "Discord.h"
#include <chrono>
#include "../DiscordSDK/include/discord_rpc.h"

void Discord::Initialize(const char* appId)
{
    DiscordEventHandlers Handle;
    memset(&Handle, 0, sizeof(Handle));
    Discord_Initialize(appId, &Handle, 1, NULL);
}

void Discord::Update(const char* detail, const char* state, const char* largeImageText, const char* largeImageKey, const char* smallImageText, const char* smallImageKey, const char* button1Label, const char* button1Url, const char* button2Label, const char* button2Url, int64_t startTimestamp)
{
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));

    discordPresence.state = state;
    discordPresence.details = detail;
    discordPresence.startTimestamp = startTimestamp;
    discordPresence.largeImageKey = largeImageKey;
    discordPresence.largeImageText = largeImageText;
    discordPresence.smallImageKey = smallImageKey;
    discordPresence.smallImageText = smallImageText;
    discordPresence.button1_label = button1Label;
    discordPresence.button1_url = button1Url;
    discordPresence.button2_label = button2Label;
    discordPresence.button2_url = button2Url;

    Discord_UpdatePresence(&discordPresence);
    Discord_RunCallbacks();
}
