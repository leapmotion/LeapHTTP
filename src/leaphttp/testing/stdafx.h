// Copyright (C) 2012-2016 Leap Motion, Inc. All rights reserved.
#pragma once

// Prevent #define min/max in windows.h
#define NOMINMAX

#if _WIN32

#include <algorithm>
#include <cstring>
#include <deque>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <chrono>
#include <thread>
#include <mutex>

#include <curl/curl.h>

#endif // _WIN32
