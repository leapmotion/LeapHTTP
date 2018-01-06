// Copyright (C) 2012-2018 Leap Motion, Inc. All rights reserved.
#include "stdafx.h"
#include "NetworkServices.h"

#include <curl/curl.h>

NetworkServices::NetworkServices() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

NetworkServices::~NetworkServices() {
  curl_global_cleanup();
}
