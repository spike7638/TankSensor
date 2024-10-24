#pragma once

#ifndef WEB_h
#define WEB_h

#define FILESYSTEM LittleFS
// timeout for connecting to a Wifi network
#define TIMEOUT 15000

#include "AsyncFsWebServer.h"
#include <FS.h>
#include <LittleFS.h>
#include "esp_task_wdt.h"

void webInit(bool show_editor); // initialize the system's Wifi connection, start up a web server, and optionally show a "/edit" page for that server

#endif
