// Copyright (c) 2010 - 2015 Leap Motion. All rights reserved. Proprietary and confidential.
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
#include <openssl/ssl.h>

#endif // _WIN32
