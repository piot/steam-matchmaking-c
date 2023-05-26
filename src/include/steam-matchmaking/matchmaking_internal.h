/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef STEAM_MATCHMAKING_MATCHMAKING_INTERNAL_H
#define STEAM_MATCHMAKING_MATCHMAKING_INTERNAL_H

#include <steam-api/steam_api.h>

typedef void* ISteamMatchmaking;
typedef SteamId LobbySteamId;

enum { k_iSteamMatchmakingCallbacks = 500 };

typedef struct LobbyEnter_t {
    enum { k_lobbyEnteriCallback = k_iSteamMatchmakingCallbacks + 4 };
    SteamId m_ulSteamIDLobby;
    uint32_t m_rgfChatPermissions;
    SteamBool m_bLocked;
    uint32_t m_EChatRoomEnterResponse;
} LobbyEnter_t;

typedef struct LobbyChatUpdate_t {
    enum { k_lobbyChatUpdateiCallback = k_iSteamMatchmakingCallbacks + 6 };
    SteamId m_ulSteamIDLobby;
    SteamId m_ulSteamIDUserChanged;
    SteamId m_ulSteamIDMakingChange;
    uint32_t m_rgfChatMemberStateChange;
} LobbyChatUpdate_t;

typedef struct LobbyMatchList_t {
    enum { k_lobbyMatchListiCallback = k_iSteamMatchmakingCallbacks + 6 };
    uint32_t lobbyMatchCount;
} LobbyMatchList_t;

typedef enum EChatRoomEnterResponse {
    k_EChatReserved,
    k_EChatRoomEnterResponseSuccess,
    k_EChatRoomEnterResponseDoesntExist,
    k_EChatRoomEnterResponseNotAllowed,
    k_EChatRoomEnterResponseFull,
    k_EChatRoomEnterResponseError,
    k_EChatRoomEnterResponseBanned,
    k_EChatRoomEnterResponseLimited,
    k_EChatRoomEnterResponseClanDisabled,
    k_EChatRoomEnterResponseCommunityBan,
    k_EChatRoomEnterResponseMemberBlockedYou,
    k_EChatRoomEnterResponseYouBlockedMember
} EChatRoomEnterResponse;

typedef enum ELobbyType {
    k_ELobbyTypePrivate,
    k_ELobbyTypeFriendsOnly,
    k_ELobbyTypePublic,
    k_ELobbyTypeInvisible,
    k_ELobbyTypePrivateUnique,
} ELobbyType;

typedef enum ELobbyComparison {
    k_ELobbyComparisonEqualToOrLessThan = -2,
    k_ELobbyComparisonLessThan = -1,
    k_ELobbyComparisonEqual = 0,
    k_ELobbyComparisonGreaterThan = 1,
    k_ELobbyComparisonEqualToOrGreaterThan = 2,
    k_ELobbyComparisonNotEqual = 3,
} ELobbyComparison;

typedef enum ELobbyDistanceFilter {
    k_ELobbyDistanceFilterClose = 0,
    k_ELobbyDistanceFilterDefault = 1,
    k_ELobbyDistanceFilterFar = 2,
    k_ELobbyDistanceFilterWorldwide = 3
} ELobbyDistanceFilter;

typedef ISteamMatchmaking*(S_CALLTYPE* SteamAPI_SteamMatchmaking_v009)(void);
typedef SteamU64(S_CALLTYPE* SteamAPI_ISteamMatchmaking_JoinLobby)(ISteamMatchmaking* matchmaking,
                                                                   LobbySteamId steamIDLobby);
typedef void(S_CALLTYPE* SteamAPI_ISteamMatchmaking_LeaveLobby)(ISteamMatchmaking* matchmaking, LobbySteamId lobbyId);
typedef SteamU64(S_CALLTYPE* SteamAPI_ISteamMatchmaking_GetLobbyMemberByIndex)(ISteamMatchmaking* matchmaking,
                                                                               LobbySteamId lobbyId, SteamInt index);
typedef SteamInt(S_CALLTYPE* SteamAPI_ISteamMatchmaking_GetNumLobbyMembers)(ISteamMatchmaking* matchmaking,
                                                                            LobbySteamId lobbyId);
typedef SteamAPICall_t(S_CALLTYPE* SteamAPI_ISteamMatchmaking_CreateLobby)(ISteamMatchmaking* matchmaking,
                                                                           ELobbyType eLobbyType,
                                                                           SteamInt cbMaxMembers);

typedef SteamBool(S_CALLTYPE* SteamAPI_ISteamMatchmaking_SetLobbyJoinable)(ISteamMatchmaking* matchmaking,
                                                                           LobbySteamId lobbyId, SteamBool joinable);
typedef SteamBool(S_CALLTYPE* SteamAPI_ISteamMatchmaking_SetLobbyType)(ISteamMatchmaking* matchmaking,
                                                                       LobbySteamId lobbyId,
                                                                       ELobbyType lobbyType);

typedef SteamBool(S_CALLTYPE* SteamAPI_ISteamMatchmaking_SetLobbyOwner)(ISteamMatchmaking* matchmaking,
                                                                        LobbySteamId lobbyId, SteamId userId);

typedef SteamId(S_CALLTYPE* SteamAPI_ISteamMatchmaking_GetLobbyOwner)(ISteamMatchmaking* matchmaking,
                                                                      LobbySteamId lobbyId);

// ------------------------------------- Find Lobbies --------------------------------------
typedef void(S_CALLTYPE* SteamAPI_ISteamMatchmaking_AddRequestLobbyListDistanceFilter)(
    ISteamMatchmaking* matchmaking, ELobbyDistanceFilter distanceFilter);
typedef void(S_CALLTYPE* SteamAPI_ISteamMatchmaking_AddRequestLobbyListFilterSlotsAvailable)(
    ISteamMatchmaking* matchmaking, SteamInt minimumNumberOfSlotsAvailable);
typedef void(S_CALLTYPE* SteamAPI_ISteamMatchmaking_AddRequestLobbyListNearValueFilter)(ISteamMatchmaking* matchmaking,
                                                                                        const char* key,
                                                                                        SteamInt valueToBeCloseTo);
typedef void(S_CALLTYPE* SteamAPI_ISteamMatchmaking_AddRequestLobbyListNumericalFilter)(ISteamMatchmaking* matchmaking,
                                                                                        const char* key, SteamInt value,
                                                                                        ELobbyComparison comparison);
typedef void(S_CALLTYPE* SteamAPI_ISteamMatchmaking_AddRequestLobbyListStringFilter)(ISteamMatchmaking* matchmaking,
                                                                                     const char* key, const char* value,
                                                                                     ELobbyComparison comparison);
typedef void(S_CALLTYPE* SteamAPI_ISteamMatchmaking_AddRequestLobbyListResultCountFilter)(
    ISteamMatchmaking* matchmaking, SteamInt maxNumberOfResults);

typedef SteamAPICall_t(S_CALLTYPE* SteamAPI_ISteamMatchmaking_RequestLobbyList)(ISteamMatchmaking* matchmaking);
typedef LobbySteamId(S_CALLTYPE* SteamAPI_ISteamMatchmaking_GetLobbyByIndex)(ISteamMatchmaking* matchmaking,
                                                                             SteamInt index);

// ------------------------------------- Lobby Data --------------------------------------
typedef SteamBool(S_CALLTYPE* SteamAPI_ISteamMatchmaking_SetLobbyData)(ISteamMatchmaking* matchmaking,
                                                                       LobbySteamId lobbyId, const char* key,
                                                                       const char* value);
typedef const char*(S_CALLTYPE* SteamAPI_ISteamMatchmaking_GetLobbyData)(ISteamMatchmaking* matchmaking,
                                                                         LobbySteamId lobbyId, const char* key);
typedef SteamBool(S_CALLTYPE* SteamAPI_ISteamMatchmaking_DeleteLobbyData)(ISteamMatchmaking* matchmaking,
                                                                          LobbySteamId lobbyId, const char* key);

typedef SteamBool(S_CALLTYPE* SteamAPI_ISteamMatchmaking_RequestLobbyData)(
    ISteamMatchmaking* matchmaking,
    LobbySteamId lobbyId); // Only used to refresh lobby data from a lobby list

typedef SteamInt(S_CALLTYPE* SteamAPI_ISteamMatchmaking_GetLobbyDataCount)(ISteamMatchmaking* matchmaking,
                                                                           LobbySteamId lobbyId);
typedef SteamBool(S_CALLTYPE* SteamAPI_ISteamMatchmaking_GetLobbyDataByIndex)(
    ISteamMatchmaking* matchmaking, LobbySteamId lobbyId, SteamInt index, char* targetKey, SteamInt maxTargetKeySize,
    char* targetValue, SteamInt maxTargetValueSize);

#endif
