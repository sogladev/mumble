// Copyright 2021-2023 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "Game.h"

#include <sstream>

Game::Game(const procid_t id, const std::string &name) : m_proc(id, name) {
}

Mumble_PositionalDataErrorCode Game::init() {
	// Check if we can read the player state to verify memory access
	// Disable below; m_ok = true is never set for "wine-preloader"
	// if (!m_proc.isOk()) {
		// return MUMBLE_PDEC_ERROR_TEMP;
	// }
	const Modules &modules = m_proc.modules();
	const auto iter        = modules.find("Wow.exe");
	if (iter == modules.cend()) {
		return MUMBLE_PDEC_ERROR_TEMP;
	}
	
	try {
		uint8_t state = m_proc.peek< uint8_t >(STATE_ADDRESS);
		return MUMBLE_PDEC_OK;
	} catch (...) {
		return MUMBLE_PDEC_ERROR_TEMP;
	}
}

uint8_t Game::getPlayerState() const {
	return m_proc.peek< uint8_t >(STATE_ADDRESS);
}

Vector3f Game::getAvatarPosition() const {
	return m_proc.peek< Vector3f >(AVATAR_POS_ADDRESS);
}

float Game::getAvatarHeading() const {
	return m_proc.peek< float >(AVATAR_HEADING_ADDRESS);
}

Vector3f Game::getCameraPosition() const {
	return m_proc.peek< Vector3f >(CAMERA_POS_ADDRESS);
}

Vector3f Game::getCameraFront() const {
	return m_proc.peek< Vector3f >(CAMERA_FRONT_ADDRESS);
}

Vector3f Game::getCameraTop() const {
	return m_proc.peek< Vector3f >(CAMERA_TOP_ADDRESS);
}

const char *Game::getPlayerName() const {
	static char playerName[50] = { 0 };
	try {
		m_proc.peek(PLAYER_ADDRESS, playerName, sizeof(playerName));
		playerName[sizeof(playerName) - 1] = '\0';
		return playerName;
	} catch (...) {
		playerName[0] = '\0';
		return nullptr;
	}
}

uint32_t Game::getMapId() const {
	return m_proc.peek< uint32_t >(MAPID_ADDRESS);
}

uint64_t Game::getLeaderGuid() const {
	return m_proc.peek< uint64_t >(LEADERGUID_ADDRESS);
}

const std::string &Game::getIdentity() {
	std::ostringstream stream;

	try {
		const char *playerName = getPlayerName();

		stream << "Name: " << (playerName ? playerName : "Unknown") << '\n';

		m_identity = stream.str();
	} catch (...) {
		m_identity = "Error reading player data";
	}

	return m_identity;
}

const std::string &Game::getContext() {
	std::ostringstream stream;

	try {
		uint32_t mapId = getMapId();

		stream << "Map ID: " << mapId << '\n';
		stream << "Leader GUID: " << getLeaderGuid();

		m_context = stream.str();
	} catch (...) {
		m_context = "Error reading context data";
	}

	return m_context;
}