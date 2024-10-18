#pragma once

#ifndef WEB_h
#define WEB_h

#define FILESYSTEM LittleFS

// Turn off the "setup" page for the ESP-32
//#define ESP_FS_WS_SETUP 0
//#define ESP_FS_WS_SETUP_HTM     0

#include "AsyncFsWebServer.h"
#include <FS.h>
#include <LittleFS.h>
/////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
////////////////////////////  HTTP Request Handlers  ////////////////////////////////////
void getDefaultValue (AsyncWebServerRequest *request);
void handleSensorSettingsForm(AsyncWebServerRequest *request);
void handleForm2(AsyncWebServerRequest *request);

////////////////////////////////  Filesystem  /////////////////////////////////////////
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
bool startFilesystem();

void webInit(bool show_editor);

#endif
/////////////////////////////////////////////////////////////////