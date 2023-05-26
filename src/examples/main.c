/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <clog/console.h>
#include <steam-api/steam_api.h>
#include <steam-matchmaking/matchmaking.h>
#include <unistd.h>

clog_config g_clog;

typedef struct ExampleData {
    int data;
} ExampleData;

static void lobbyEnter(void* _self, SteamId lobbySteamId, uint32_t chatPermissions, uint32_t enterResponse)
{
    ExampleData* self = (ExampleData*) _self;

    CLOG_INFO("lobbyEnter callback %d", self->data, lobbySteamId)
}

static void lobbyChatUpdate(void* _self, SteamId lobbySteamId)
{
    ExampleData* self = (ExampleData*) _self;

    CLOG_INFO("lobbyEnter callback %d", self->data, lobbySteamId)
}

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    g_clog.log = clog_console;
    g_clog.level = CLOG_TYPE_VERBOSE;

    SteamApi api;
    steamApiInit(&api);

    SteamMatchMaking matchMaking;
    steamMatchMakingInit(&matchMaking, &api);

    ExampleData data;
    data.data = 42;

    if (true) {
        steamMatchMakingRegisterLobbyEnterCallback(&matchMaking, &data, lobbyEnter);
        steamMatchMakingRegisterLobbyChatUpdateCallback(&matchMaking, &data, lobbyChatUpdate);

        steamMatchMakingCreateLobby(&matchMaking, k_ELobbyTypeFriendsOnly, 2);
    }

    uint64_t tickCount = 0;
    while (true) {
        steamApiUpdate(&api);
        usleep(16 * 1000);
        tickCount++;
        if (tickCount == 60) {
            steamMatchMakingAddRequestLobbyListFilterSlotsAvailable(&matchMaking, 1);
            steamMatchMakingAddRequestLobbyListResultCountFilter(&matchMaking, 5);
            steamMatchMakingRequestLobbyList(&matchMaking);
        }
    }
}
