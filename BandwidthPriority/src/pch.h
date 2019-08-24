// Precompiled headers go in here
#pragma once

#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <winsock.h>
// iphlpapi must be after winsock
#include <iphlpapi.h>
#include <chrono>

// Data structures
#include <string>
#include <vector>
#include <unordered_map>

// Windows API
#include <Windows.h>

// WinDivert
#include "windivert.h"

// My commmon headers
#include "Log.h"