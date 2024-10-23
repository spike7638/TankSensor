#pragma once

#ifndef WEB_h
#define WEB_h

#define FILESYSTEM LittleFS

#include "AsyncFsWebServer.h"
#include <FS.h>
#include <LittleFS.h>

void webInit(bool show_editor); // initialize the system's Wifi connection, start up a web server, and optionally show a "/edit" page for that server

#endif
