// Copyright 2021-2023 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef WOW3_GAME_H_
#define WOW3_GAME_H_

#include "ProcessWindows.h"
#define MUMBLE_PLUGIN_NO_DEFAULT_FUNCTION_DEFINITIONS
#include "MumblePlugin.h"
#undef MUMBLE_PLUGIN_NO_DEFAULT_FUNCTION_DEFINITIONS

#include <string>
#include <array>

using Vector3f = std::array<float, 3>;

constexpr std::string_view WOW_EXE = "Wow.exe"; // exact

class Game {
public:
    Mumble_PositionalDataErrorCode init();
	//
    // Read game state
    uint8_t getPlayerState() const;
    Vector3f getAvatarPosition() const;
    float getAvatarHeading() const;
    Vector3f getCameraPosition() const;
    Vector3f getCameraFront() const;
    Vector3f getCameraTop() const;
    const char* getPlayerName() const;
    uint32_t getMapId() const;
    uint64_t getLeaderGuid() const;

    // Generate identity string
    const std::string& getIdentity();

    // Generate context string
    const std::string& getContext();

    Game(procid_t id, const std::string &name);

protected:
    // Static memory addresses for WoW
    static constexpr procptr_t STATE_ADDRESS          = 0x00BD0792;
    static constexpr procptr_t AVATAR_POS_ADDRESS     = 0x00ADF4E4;
    static constexpr procptr_t AVATAR_HEADING_ADDRESS = 0x00BEBA70;
    static constexpr procptr_t CAMERA_POS_ADDRESS     = 0x00ADF4E4;
    static constexpr procptr_t CAMERA_FRONT_ADDRESS   = 0x00ADF5F0;
    static constexpr procptr_t CAMERA_TOP_ADDRESS     = 0x00ADF554;
    static constexpr procptr_t PLAYER_ADDRESS         = 0x00C79D18;
    static constexpr procptr_t MAPID_ADDRESS          = 0x00AB63BC;
	static constexpr procptr_t LEADERGUID_ADDRESS     = 0x00BD1968;

	std::string m_identity;
    std::string m_context;
    ProcessWindows m_proc;
};

#endif