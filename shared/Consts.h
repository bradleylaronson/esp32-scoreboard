#pragma once

// ============================================================================
// FIRMWARE VERSION
// ============================================================================

// Semantic versioning: MAJOR.MINOR.PATCH
#define FIRMWARE_VERSION_MAJOR 1
#define FIRMWARE_VERSION_MINOR 0
#define FIRMWARE_VERSION_PATCH 0

// Stage identifier
#define FIRMWARE_STAGE "Stage 1"

// Build string (constructed from components above)
#define FIRMWARE_VERSION "v1.0.0"
#define FIRMWARE_FULL_VERSION FIRMWARE_STAGE " - " FIRMWARE_VERSION

// Compile date/time
#define FIRMWARE_BUILD_DATE __DATE__
#define FIRMWARE_BUILD_TIME __TIME__
