// Copyright (c) 2010 - 2015 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "NetworkServices.h"

#include <curl/curl.h>

NetworkServices::NetworkServices() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

NetworkServices::~NetworkServices() {
  curl_global_cleanup();
}
