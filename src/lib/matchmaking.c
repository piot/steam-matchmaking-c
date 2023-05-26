/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include "clog/clog.h"
#include <atheneum/atheneum.h>
#include <steam-matchmaking/matchmaking.h>

#define BIND(formalName, fieldName)                                                                                    \
    self->fieldName = (formalName) atheneumAddress(atheneum, #formalName);                                             \
    if (self->fieldName == 0) {                                                                                        \
        CLOG_ERROR("could not find '%s'", #formalName)                                                                 \
        return -1;                                                                                                     \
    }

static int steamMatchmakingInitFunctions(SteamMatchMakingFunctions* self, Atheneum* atheneum)
{
    BIND(SteamAPI_SteamMatchmaking_v009, createMatchmakingV009)
    BIND(SteamAPI_ISteamMatchmaking_JoinLobby, joinLobby)
    BIND(SteamAPI_ISteamMatchmaking_LeaveLobby, leaveLobby)
    BIND(SteamAPI_ISteamMatchmaking_CreateLobby, createLobby)
    BIND(SteamAPI_ISteamMatchmaking_GetLobbyMemberByIndex, getLobbyMemberByIndex)
    BIND(SteamAPI_ISteamMatchmaking_GetNumLobbyMembers, getNumLobbyMembers)
    BIND(SteamAPI_ISteamMatchmaking_AddRequestLobbyListDistanceFilter, addRequestLobbyListDistanceFilter)
    BIND(SteamAPI_ISteamMatchmaking_AddRequestLobbyListFilterSlotsAvailable, addRequestLobbyListFilterSlotsAvailable)
    BIND(SteamAPI_ISteamMatchmaking_AddRequestLobbyListNearValueFilter, addRequestLobbyListNearValueFilter)
    BIND(SteamAPI_ISteamMatchmaking_AddRequestLobbyListNumericalFilter, addRequestLobbyListNumericalFilter)
    BIND(SteamAPI_ISteamMatchmaking_AddRequestLobbyListStringFilter, addRequestLobbyListStringFilter)
    BIND(SteamAPI_ISteamMatchmaking_AddRequestLobbyListResultCountFilter, addRequestLobbyListResultCountFilter)
    BIND(SteamAPI_ISteamMatchmaking_RequestLobbyList, requestLobbyList)
    BIND(SteamAPI_ISteamMatchmaking_SetLobbyJoinable, setLobbyJoinable)
    BIND(SteamAPI_ISteamMatchmaking_SetLobbyType, setLobbyType)
    BIND(SteamAPI_ISteamMatchmaking_SetLobbyOwner, setLobbyOwner)
    BIND(SteamAPI_ISteamMatchmaking_GetLobbyOwner, getLobbyOwner)
    BIND(SteamAPI_ISteamMatchmaking_GetLobbyByIndex, getLobbyByIndex)
    BIND(SteamAPI_ISteamMatchmaking_SetLobbyData, setLobbyData)
    BIND(SteamAPI_ISteamMatchmaking_GetLobbyData, getLobbyData)
    BIND(SteamAPI_ISteamMatchmaking_GetLobbyDataByIndex, getLobbyDataByIndex)
    BIND(SteamAPI_ISteamMatchmaking_DeleteLobbyData, deleteLobbyData)
    BIND(SteamAPI_ISteamMatchmaking_RequestLobbyData, requestLobbyData)
    BIND(SteamAPI_ISteamMatchmaking_GetLobbyDataCount, getLobbyDataCount)

    return 0;
}

int steamMatchMakingInit(SteamMatchMaking* self, SteamApi* api)
{
    int functionsResult = steamMatchmakingInitFunctions(&self->functions, &api->atheneum);
    if (functionsResult < 0) {
        return functionsResult;
    }

    self->api = api;
    self->matchmaking = self->functions.createMatchmakingV009();

    CLOG_INFO("matchmaking create %p", (void*) self->matchmaking);

    return 0;
}

static const char* chatRoomEnterResponse(EChatRoomEnterResponse response)
{
    const char* descriptions[] = {"reserved",     "Success",      "DoesntExist",      "NotAllowed",
                                  "Full",         "Error",        "Banned",           "Limited",
                                  "ClanDisabled", "CommunityBan", "MemberBlockedYou", "YouBlockedMember"};

    if (response >= 0 && response < k_EChatRoomEnterResponseYouBlockedMember) {
        return descriptions[response];
    }

    return "unknown response";
}

static void callbackLobbyEnterRun(void* _self, void* pvParam)
{
    LobbyEnterCallback* self = (LobbyEnterCallback*) _self;
    const LobbyEnter_t* param = (const LobbyEnter_t*) pvParam;

    CLOG_INFO("response was '%s'", chatRoomEnterResponse(param->m_EChatRoomEnterResponse))

    if (param->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess) {
    }

    CLOG_INFO("lobbyEnterRun(%d) %016llX", self->cookie, param->m_ulSteamIDLobby)

    if (self->userFn) {
        self->userFn(self->userData, param->m_ulSteamIDLobby, param->m_rgfChatPermissions,
                     param->m_EChatRoomEnterResponse);
    }
}

static void callbackLobbyEnterResult(void* _self, void* pvParam, SteamBool bIOFailure,
                                     SteamAPICall_t handleSteamApiCall)
{
    LobbyEnterCallback* self = (LobbyEnterCallback*) _self;
    const LobbyEnter_t* param = (const LobbyEnter_t*) pvParam;
    CLOG_INFO("response was '%s'", chatRoomEnterResponse(param->m_EChatRoomEnterResponse))
    CLOG_INFO("lobbyEnterResult(%d) %d %016llX", self->cookie, param->m_EChatRoomEnterResponse, param->m_ulSteamIDLobby)
}

static SteamInt callbackLobbyEnterGetSize(void)
{
    CLOG_INFO("getSize()")
    return sizeof(LobbyEnter_t);
}

int steamMatchMakingRegisterLobbyEnterCallback(SteamMatchMaking* self, void* data, SteamMatchMakingOnLobbyEnterFn fn)
{
    self->lobbyEnterCallback.vtable.runFn = callbackLobbyEnterRun;
    self->lobbyEnterCallback.vtable.getCallbackSizeBytesFn = callbackLobbyEnterGetSize;
    self->lobbyEnterCallback.vtable.resultFn = callbackLobbyEnterResult;

    self->lobbyEnterCallback.base.iCallback = k_lobbyEnteriCallback;
    self->lobbyEnterCallback.base.vtable = &self->lobbyEnterCallback.vtable;
    self->lobbyEnterCallback.base.callbackFlags = 0;
    self->lobbyEnterCallback.cookie = 42;

    self->lobbyEnterCallback.userData = data;
    self->lobbyEnterCallback.userFn = fn;

    steamApiRegisterCallback(self->api, &self->lobbyEnterCallback.base, k_lobbyEnteriCallback);

    return 0;
}

static void callbackLobbyChatUpdateRun(void* _self, void* pvParam)
{
    LobbyChatUpdateCallback* self = (LobbyChatUpdateCallback*) _self;
    const LobbyChatUpdate_t* param = (const LobbyChatUpdate_t*) pvParam;

    CLOG_INFO("lobbyChatUpdate(%d) %016llX  %016llX", self->cookie, param->m_ulSteamIDLobby,
              param->m_ulSteamIDUserChanged)

    if (self->userFn) {
        self->userFn(self->userData, param->m_ulSteamIDLobby);
    }
}

static void callbackLobbyChatUpdateResult(void* _self, void* pvParam, SteamBool bIOFailure,
                                          SteamAPICall_t handleSteamApiCall)
{
    LobbyChatUpdateCallback* self = (LobbyChatUpdateCallback*) _self;
    const LobbyChatUpdate_t* param = (const LobbyChatUpdate_t*) pvParam;
    CLOG_INFO("user changed was was '%s'", chatRoomEnterResponse(param->m_ulSteamIDUserChanged))
    CLOG_INFO("lobbyChatUpdate(%d) %016lX %016lX", self->cookie, param->m_ulSteamIDUserChanged, param->m_ulSteamIDLobby)
}

static SteamInt callbackLobbyChatUpdateGetSize(void)
{
    CLOG_INFO("getSize()")
    return sizeof(LobbyChatUpdate_t);
}

int steamMatchMakingRegisterLobbyChatUpdateCallback(SteamMatchMaking* self, void* data,
                                                    SteamMatchMakingOnLobbyChatUpdateFn fn)
{
    self->lobbyChatUpdateCallback.vtable.runFn = callbackLobbyChatUpdateRun;
    self->lobbyChatUpdateCallback.vtable.getCallbackSizeBytesFn = callbackLobbyChatUpdateGetSize;
    self->lobbyChatUpdateCallback.vtable.resultFn = callbackLobbyChatUpdateResult;

    self->lobbyChatUpdateCallback.base.iCallback = k_lobbyChatUpdateiCallback;
    self->lobbyChatUpdateCallback.base.vtable = &self->lobbyChatUpdateCallback.vtable;
    self->lobbyChatUpdateCallback.base.callbackFlags = 0;
    self->lobbyChatUpdateCallback.cookie = 42;

    self->lobbyChatUpdateCallback.userData = data;
    self->lobbyChatUpdateCallback.userFn = fn;

    steamApiRegisterCallback(self->api, &self->lobbyEnterCallback.base, k_lobbyChatUpdateiCallback);

    return 0;
}

int steamMatchMakingCreateLobby(SteamMatchMaking* self, ELobbyType eLobbyType, SteamInt cbMaxMembers)
{
    CLOG_INFO("create lobby max:%d", cbMaxMembers)
    SteamAPICall_t handle = self->functions.createLobby(self->matchmaking, eLobbyType, cbMaxMembers);

    self->lobbyEnterCallback.vtable.runFn = callbackLobbyEnterRun;
    self->lobbyEnterCallback.vtable.getCallbackSizeBytesFn = callbackLobbyEnterGetSize;
    self->lobbyEnterCallback.vtable.resultFn = callbackLobbyEnterResult;

    self->lobbyEnterCallback.base.iCallback = k_lobbyEnteriCallback;
    self->lobbyEnterCallback.base.vtable = &self->lobbyEnterCallback.vtable;
    self->lobbyEnterCallback.base.callbackFlags = 0;
    self->lobbyEnterCallback.cookie = 42;

    self->lobbyEnterCallback.userData = 0;
    self->lobbyEnterCallback.userFn = 0;

    steamApiRegisterCallResult(self->api, &self->lobbyEnterCallback.base, handle);

    CLOG_INFO("returned lobby %016llX", handle)

    return 0;
}



/// @name lobby_filter
/// All lobby filter
/// @{

/// Add filter for number of free slots
/// @param self
/// @param minimumNumberOfSlotsFree
void steamMatchMakingAddRequestLobbyListFilterSlotsAvailable(SteamMatchMaking* self, SteamInt minimumNumberOfSlotsFree)
{
    self->functions.addRequestLobbyListFilterSlotsAvailable(self->matchmaking, minimumNumberOfSlotsFree);
}


/// Add filter for value to be close to
/// @param self
/// @param key
/// @param valueToBeCloseTo
void steamMatchMakingAddRequestLobbyListNearValue(SteamMatchMaking* self, const char* key, SteamInt valueToBeCloseTo)
{
    self->functions.addRequestLobbyListNearValueFilter(self->matchmaking, key, valueToBeCloseTo);
}

/// Add filter for geographical distance
/// @param self
/// @param distance
void steamMatchMakingAddRequestLobbyListDistanceFilter(SteamMatchMaking* self, ELobbyDistanceFilter distance)
{
    self->functions.addRequestLobbyListDistanceFilter(self->matchmaking, distance);
}


/// Add filter for a number comparison
/// @param self
/// @param key
/// @param valueToMatch
/// @param comparison
void steamMatchMakingAddRequestLobbyListNumericalFilter(SteamMatchMaking* self, const char* key, SteamInt valueToMatch,
                                                        ELobbyComparison comparison)
{
    self->functions.addRequestLobbyListNumericalFilter(self->matchmaking, key, valueToMatch, comparison);
}

/// Add filter for a string comparison
/// @param self
/// @param key
/// @param valueToMatch
/// @param comparison
void steamMatchMakingAddRequestLobbyListStringFilter(SteamMatchMaking* self, const char* key, const char* valueToMatch,
                                                     ELobbyComparison comparison)
{
    self->functions.addRequestLobbyListStringFilter(self->matchmaking, key, valueToMatch, comparison);
}

/// Add filter for maximum number of results to return
/// @param self
/// @param maxNumberOfResults
void steamMatchMakingAddRequestLobbyListResultCountFilter(SteamMatchMaking* self, SteamInt maxNumberOfResults)
{
    self->functions.addRequestLobbyListResultCountFilter(self->matchmaking, maxNumberOfResults);
}

static void callbackLobbyMatchListRun(void* _self, void* pvParam)
{
    LobbyListCallback* self = (LobbyListCallback*) _self;
    const LobbyMatchList_t* param = (const LobbyMatchList_t*) pvParam;

    CLOG_INFO("lobby match list count is '%d'", param->lobbyMatchCount)

    if (self->userFn) {
        self->userFn(self->userData, param->lobbyMatchCount);
    }
}

static void callbackLobbyMatchListResult(void* _self, void* pvParam, SteamBool bIOFailure,
                                         SteamAPICall_t handleSteamApiCall)
{
    LobbyListCallback* self = (LobbyListCallback*) _self;
    const LobbyMatchList_t* param = (const LobbyMatchList_t*) pvParam;
    CLOG_INFO("lobby match list count is '%d'", param->lobbyMatchCount)
}

static SteamInt callbackLobbyMatchListGetSize(void)
{
    return sizeof(LobbyMatchList_t);
}

/// Request a lobby list conforming to the previously added filters
/// @param self
void steamMatchMakingRequestLobbyList(SteamMatchMaking* self)
{
    SteamAPICall_t handle = self->functions.requestLobbyList(self->matchmaking);
    self->lobbyListCallback.vtable.runFn = callbackLobbyMatchListRun;
    self->lobbyListCallback.vtable.getCallbackSizeBytesFn = callbackLobbyMatchListGetSize;
    self->lobbyListCallback.vtable.resultFn = callbackLobbyMatchListResult;

    self->lobbyListCallback.base.iCallback = k_lobbyMatchListiCallback;
    self->lobbyListCallback.base.vtable = &self->lobbyListCallback.vtable;
    self->lobbyListCallback.base.callbackFlags = 0;
    self->lobbyListCallback.cookie = 42;

    self->lobbyListCallback.userData = 0;
    self->lobbyListCallback.userFn = 0;

    steamApiRegisterCallResult(self->api, &self->lobbyListCallback.base, handle);
}

/// @}



/// @name lobby_data
/// All lobby data functions
/// @{

/// Sets lobby data key value pair
/// @param self
/// @param lobbyId
/// @param key
/// @param value
void steamMatchMakingSetLobbyData(SteamMatchMaking* self, LobbySteamId lobbyId, const char* key, const char* value)
{
    self->functions.setLobbyData(self->matchmaking, lobbyId, key, value);
}

/// Gets lobby data from key
/// @param self
/// @param lobbyId
/// @param key
/// @return string value
const char* steamMatchMakingGetLobbyData(SteamMatchMaking* self, LobbySteamId lobbyId, const char* key)
{
    return self->functions.getLobbyData(self->matchmaking, lobbyId, key);
}

/// Delete lobby data key and value
/// @param self
/// @param lobbyId
/// @param key
/// @return
bool steamMatchMakingDeleteLobbyData(SteamMatchMaking* self, LobbySteamId lobbyId, const char* key)
{
    return self->functions.deleteLobbyData(self->matchmaking, lobbyId, key);
}

/// Refreshes lobby data
/// @note do not call on own lobby
/// @param self
/// @param lobbyId
/// @return
bool steamMatchMakingRequestLobbyData(SteamMatchMaking* self, LobbySteamId lobbyId)
{
    return self->functions.requestLobbyData(self->matchmaking, lobbyId);
}

/// Get the number of key value pairs
/// @param self
/// @param lobbyId
/// @return
int steamMatchMakingGetLobbyDataCount(SteamMatchMaking* self, LobbySteamId lobbyId)
{
    return self->functions.getLobbyDataCount(self->matchmaking, lobbyId);
}

/// Get key value pairs by index
/// @param self
/// @param lobbyId
/// @param index
/// @param targetKey
/// @param maxKeySize
/// @param targetValue
/// @param maxTargetValueSize
/// @return
bool steamMatchMakingGetLobbyDataByIndex(SteamMatchMaking* self, LobbySteamId lobbyId, int index, char* targetKey,
                                         size_t maxKeySize, char* targetValue, size_t maxTargetValueSize)
{
    return self->functions.getLobbyDataByIndex(self->matchmaking, lobbyId, index, targetKey, (SteamInt) maxKeySize,
                                               targetValue, (SteamInt) maxTargetValueSize);
}

/// @}


/// @name lobby_state
/// All lobby state functions
/// @{

/// Set lobby to be joinable or blocked/invisible
/// Default is that joinable is true.
/// @param self
/// @param lobbyId
/// @param joinable
/// @return
bool steamMatchMakingSetLobbyJoinable(SteamMatchMaking* self, LobbySteamId lobbyId, bool joinable)
{
    return self->functions.setLobbyJoinable(self->matchmaking, lobbyId, joinable);
}

/// Set lobby type
/// @param self
/// @param lobbyId
/// @param lobbyType
/// @return
bool steamMatchMakingSetLobbyType(SteamMatchMaking* self, LobbySteamId lobbyId, ELobbyType lobbyType)
{
    return self->functions.setLobbyType(self->matchmaking, lobbyId, lobbyType);
}

/// Change the lobby owner
/// Only the current owner can issue the switch to a new owner
/// @param self
/// @param lobbyId
/// @param userId
/// @return
bool steamMatchMakingSetLobbyOwner(SteamMatchMaking* self, LobbySteamId lobbyId, SteamId userId)
{
    return self->functions.setLobbyOwner(self->matchmaking, lobbyId, userId);
}

/// Get the current lobby owner
/// @param self
/// @param lobbyId
/// @return
SteamId steamMatchMakingGetLobbyOwner(SteamMatchMaking* self, LobbySteamId lobbyId)
{
    return self->functions.getLobbyOwner(self->matchmaking, lobbyId);
}
