// Copyright 2016-2023 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "Game.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string_view>

static std::unique_ptr< Game > game;

mumble_error_t mumble_init(uint32_t) {
	return MUMBLE_STATUS_OK;
}

void mumble_shutdown() {
}

MumbleStringWrapper mumble_getName() {
	static constexpr char name[] = "World of Warcraft Wrath of the Lich King 3.3.5a";

	MumbleStringWrapper wrapper{};
	wrapper.data           = name;
	wrapper.size           = strlen(name);
	wrapper.needsReleasing = false;

	return wrapper;
}

mumble_version_t mumble_getAPIVersion() {
	return MUMBLE_PLUGIN_API_VERSION;
}

void mumble_registerAPIFunctions(void *) {
}

void mumble_releaseResource(const void *) {
}

mumble_version_t mumble_getVersion() {
	return { 2, 0, 0 };
}

MumbleStringWrapper mumble_getAuthor() {
	static constexpr char author[] = "MumbleDevelopers";

	MumbleStringWrapper wrapper{};
	wrapper.data           = author;
	wrapper.size           = strlen(author);
	wrapper.needsReleasing = false;

	return wrapper;
}

MumbleStringWrapper mumble_getDescription() {
	static constexpr char description[] = "Provides positional audio functionality for World of Warcraft. "
										  "Identity is provided.";

	MumbleStringWrapper wrapper{};
	wrapper.data           = description;
	wrapper.size           = strlen(description);
	wrapper.needsReleasing = false;

	return wrapper;
}

uint32_t mumble_getFeatures() {
	return MUMBLE_FEATURE_POSITIONAL;
}

#ifndef _WIN32
// Helper function to check if a Wine process is running WoW
bool isWineRunningWow(uint64_t pid) {
	char cmdlinePath[256];
	char buffer[512];
	snprintf(cmdlinePath, sizeof(cmdlinePath), "/proc/%llu/cmdline", static_cast< unsigned long long >(pid));

	FILE *cmdlineFile = fopen(cmdlinePath, "r");
	if (!cmdlineFile) {
		return false;
	}

	const size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, cmdlineFile);
	fclose(cmdlineFile);

	if (bytesRead == 0) {
		return false;
	}
	buffer[bytesRead] = '\0';

	return (strstr(buffer, WOW_EXE.data()) != nullptr);
}
#endif

uint8_t mumble_initPositionalData(const char *const *programNames, const uint64_t *programPIDs, size_t programCount) {
	Mumble_PositionalDataErrorCode ret = MUMBLE_PDEC_ERROR_TEMP;

	for (size_t i = 0; i < programCount; ++i) {
#ifdef _WIN32
		if (_stricmp(programNames[i], WOW_EXE.data()) != 0) {
			continue;
		}
#else
		if (!isWineRunningWow(programPIDs[i])) {
			continue;
		}
#endif

		game = std::make_unique< Game >(programPIDs[i], programNames[i]);

		ret = game->init();
		if (ret == MUMBLE_PDEC_OK) {
			// Check if we can get player state
			if (const uint8_t state = game->getPlayerState(); state != 1) { // 1 is in-game state
				ret = MUMBLE_PDEC_ERROR_TEMP;
			}
		}

		if (ret != MUMBLE_PDEC_OK) {
			game.reset();
		}

		break;
	}

	return ret;
}

void mumble_shutdownPositionalData() {
	game.reset();
}

bool mumble_fetchPositionalData(float *avatarPos, float *avatarDir, float *avatarAxis, float *cameraPos,
								float *cameraDir, float *cameraAxis, const char **contextPtr,
								const char **identityPtr) {
	// Initialize all vectors to zero
	std::fill_n(avatarPos, 3, 0.f);
	std::fill_n(avatarDir, 3, 0.f);
	std::fill_n(avatarAxis, 3, 0.f);
	std::fill_n(cameraPos, 3, 0.f);
	std::fill_n(cameraDir, 3, 0.f);
	std::fill_n(cameraAxis, 3, 0.f);

	// Verify that the player is in-game (state == 1)
	if (const uint8_t playerState = game->getPlayerState(); playerState != 1) {
		// Return true to keep trying even when not in game
		*contextPtr  = "{}";
		*identityPtr = "{}";
		return true;
	}

	// Get avatar position
	const Vector3f avatarPosition = game->getAvatarPosition();
	// WoW -> Mumble: X=Z, Y=-X, Z=Y
	avatarPos[0] = -avatarPosition[1];
	avatarPos[1] = avatarPosition[2];
	avatarPos[2] = avatarPosition[0];
	for (int i = 0; i < 3; ++i) {
		avatarPos[i] *= 0.9144f; // Scale yard to meter
	}

	// Get camera position
	// WoW -> Mumble: X=Z, Y=-X, Z=Y
	const Vector3f cameraPosition = game->getCameraPosition();
	cameraPos[0]                  = -cameraPosition[1];
	cameraPos[1]                  = cameraPosition[2];
	cameraPos[2]                  = cameraPosition[0];

	// Get avatar direction from heading
	const float avatarHeading = game->getAvatarHeading();
	avatarDir[0]              = -sinf(avatarHeading);
	avatarDir[1]              = 0.0f;
	avatarDir[2]              = cosf(avatarHeading);

	// Avatar axis (up vector)
	// WoW -> Mumble: X=Z, Y=-X, Z=Y
	avatarAxis[0] = 0.0f;
	avatarAxis[1] = 1.0f;
	avatarAxis[2] = 0.0f;

	// Camera direction (front vector)
	// WoW -> Mumble: X=Z, Y=-X, Z=Y
	cameraDir[0] = -sinf(avatarHeading); // Use avatar heading for simplicity
	cameraDir[1] = 0.0f;
	cameraDir[2] = cosf(avatarHeading);

	// Camera axis (up vector)
	// WoW -> Mumble: X=Z, Y=-X, Z=Y
	const Vector3f cameraTop = game->getCameraTop();
	cameraAxis[0]            = -cameraTop[1];
	cameraAxis[1]            = cameraTop[2];
	cameraAxis[2]            = cameraTop[0];

	// Get identity string
	*identityPtr = game->getIdentity().c_str();

	// Create context string
	*contextPtr = game->getContext().c_str();

	return true;
}
