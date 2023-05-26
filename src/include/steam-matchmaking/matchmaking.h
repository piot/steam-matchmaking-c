/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef STEAM_MATCHMAKING_H
#define STEAM_MATCHMAKING_H

#include <stdbool.h>
#include <steam-matchmaking/matchmaking_internal.h>

typedef void (*SteamMatchMakingOnLobbyEnterFn)(void*, SteamId lobbySteamId, uint32_t chatPermissions,
                                               uint32_t enterResponse);
typedef void (*SteamMatchMakingOnLobbyChatUpdateFn)(void*, SteamId lobbySteamId);
typedef void (*SteamMatchMakingOnLobbyListFn)(void*, uint32_t lobbyCountFound);

struct Atheneum;

typedef struct SteamMatchMakingFunctions {
    SteamAPI_SteamMatchmaking_v009 createMatchmakingV009;
    SteamAPI_ISteamMatchmaking_JoinLobby joinLobby;
    SteamAPI_ISteamMatchmaking_LeaveLobby leaveLobby;
    SteamAPI_ISteamMatchmaking_CreateLobby createLobby;
    SteamAPI_ISteamMatchmaking_GetLobbyMemberByIndex getLobbyMemberByIndex;
    SteamAPI_ISteamMatchmaking_GetNumLobbyMembers getNumLobbyMembers;
    SteamAPI_ISteamMatchmaking_AddRequestLobbyListDistanceFilter addRequestLobbyListDistanceFilter;
    SteamAPI_ISteamMatchmaking_AddRequestLobbyListFilterSlotsAvailable addRequestLobbyListFilterSlotsAvailable;
    SteamAPI_ISteamMatchmaking_AddRequestLobbyListNearValueFilter addRequestLobbyListNearValueFilter;
    SteamAPI_ISteamMatchmaking_AddRequestLobbyListNumericalFilter addRequestLobbyListNumericalFilter;
    SteamAPI_ISteamMatchmaking_AddRequestLobbyListStringFilter addRequestLobbyListStringFilter;
    SteamAPI_ISteamMatchmaking_AddRequestLobbyListResultCountFilter addRequestLobbyListResultCountFilter;
    SteamAPI_ISteamMatchmaking_RequestLobbyList requestLobbyList;
    SteamAPI_ISteamMatchmaking_SetLobbyJoinable setLobbyJoinable;
    SteamAPI_ISteamMatchmaking_SetLobbyType setLobbyType;
    SteamAPI_ISteamMatchmaking_SetLobbyOwner setLobbyOwner;
    SteamAPI_ISteamMatchmaking_GetLobbyOwner getLobbyOwner;
    SteamAPI_ISteamMatchmaking_GetLobbyByIndex getLobbyByIndex;
    SteamAPI_ISteamMatchmaking_SetLobbyData setLobbyData;
    SteamAPI_ISteamMatchmaking_GetLobbyData getLobbyData;
    SteamAPI_ISteamMatchmaking_GetLobbyDataByIndex getLobbyDataByIndex;
    SteamAPI_ISteamMatchmaking_DeleteLobbyData deleteLobbyData;
    SteamAPI_ISteamMatchmaking_RequestLobbyData requestLobbyData;
    SteamAPI_ISteamMatchmaking_GetLobbyDataCount getLobbyDataCount;
} SteamMatchMakingFunctions;

typedef struct LobbyEnterCallback {
    CCallbackBase base;
    CallbackVTable vtable;
    void* userData;
    SteamMatchMakingOnLobbyEnterFn userFn;
    int cookie;
} LobbyEnterCallback;

typedef struct LobbyListCallback {
    CCallbackBase base;
    CallbackVTable vtable;
    void* userData;
    SteamMatchMakingOnLobbyListFn userFn;
    int cookie;
} LobbyListCallback;

typedef struct LobbyChatUpdateCallback {
    CCallbackBase base;
    CallbackVTable vtable;
    void* userData;
    SteamMatchMakingOnLobbyChatUpdateFn userFn;
    int cookie;
} LobbyChatUpdateCallback;

typedef struct SteamMatchMaking {
    ISteamMatchmaking* matchmaking;
    SteamMatchMakingFunctions functions;
    SteamApi* api;
    LobbyEnterCallback lobbyEnterCallback;
    LobbyListCallback lobbyListCallback;
    LobbyChatUpdateCallback lobbyChatUpdateCallback;
} SteamMatchMaking;

int steamMatchMakingInit(SteamMatchMaking* self, SteamApi* api);
int steamMatchMakingRegisterLobbyEnterCallback(SteamMatchMaking* self, void* data, SteamMatchMakingOnLobbyEnterFn fn);
int steamMatchMakingRegisterLobbyChatUpdateCallback(SteamMatchMaking* self, void* data,
                                                    SteamMatchMakingOnLobbyChatUpdateFn fn);
int steamMatchMakingCreateLobby(SteamMatchMaking* self, ELobbyType eLobbyType, SteamInt cbMaxMembers);
void steamMatchMakingAddRequestLobbyListFilterSlotsAvailable(SteamMatchMaking* self, SteamInt minimumNumberOfSlotsFree);
void steamMatchMakingAddRequestLobbyListNearValue(SteamMatchMaking* self, const char* key, SteamInt valueToBeCloseTo);
void steamMatchMakingAddRequestLobbyListDistanceFilter(SteamMatchMaking* self, ELobbyDistanceFilter distance);
void steamMatchMakingAddRequestLobbyListNumericalFilter(SteamMatchMaking* self, const char* key, SteamInt valueToMatch,
                                                        ELobbyComparison comparison);
void steamMatchMakingAddRequestLobbyListStringFilter(SteamMatchMaking* self, const char* key, const char* valueToMatch,
                                                     ELobbyComparison comparison);
void steamMatchMakingAddRequestLobbyListResultCountFilter(SteamMatchMaking* self, SteamInt maxNumberOfResults);
void steamMatchMakingRequestLobbyList(SteamMatchMaking* self);


void steamMatchMakingSetLobbyData(SteamMatchMaking* matchmaking,
                                  LobbySteamId lobbyId, const char* key,
                                  const char* value);

const char* steamMatchMakingGetLobbyData(SteamMatchMaking* matchmaking,
                                  LobbySteamId lobbyId, const char* key);

bool steamMatchMakingDeleteLobbyData(SteamMatchMaking* matchmaking, LobbySteamId lobbyId, const char* key);

bool steamMatchMakingRequestLobbyData(SteamMatchMaking* matchmaking, LobbySteamId lobbyId);
int steamMatchMakingGetLobbyDataCount(SteamMatchMaking* matchmaking, LobbySteamId lobbyId);
bool steamMatchMakingGetLobbyDataByIndex(SteamMatchMaking* matchmaking, LobbySteamId lobbyId, int index, char* targetKey, size_t maxKeySize, char* targetValue, size_t maxTargetValueSize);


bool steamMatchMakingSetLobbyJoinable(SteamMatchMaking* self, LobbySteamId lobbyId, bool joinable);
bool steamMatchMakingSetLobbyType(SteamMatchMaking* self, LobbySteamId lobbyId, ELobbyType lobbyType);
bool steamMatchMakingSetLobbyOwner(SteamMatchMaking* self, LobbySteamId lobbyId, SteamId userId);
SteamId steamMatchMakingGetLobbyOwner(SteamMatchMaking* self, LobbySteamId lobbyId);



#endif
