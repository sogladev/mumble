# Copyright 2020-2023 The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

# Add the include directories for dependencies
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}"       # For wow3.cpp and Game.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/.."   # For ProcessWindows.h, ProcessBase.h, etc.
)

add_library(wow3 SHARED
	"wow3.cpp"
	"Game.cpp"

	"../Module.cpp"
	"../ProcessBase.cpp"
	"../ProcessWindows.cpp"
)

if(WIN32)
	target_sources(wow3 PRIVATE "../HostWindows.cpp")
else()
	target_sources(wow3 PRIVATE "../HostLinux.cpp")
endif()

# Define macros and include required headers
target_compile_definitions(wow3 PRIVATE 
    _USE_MATH_DEFINES   # For M_PI and other math constants
)

# Ensure required headers are included
target_compile_options(wow3 PRIVATE 
    -include cfenv      # For floating-point exception handling
)

add_custom_target(wow3_target DEPENDS wow3)
