#ifndef FB_ClientS_H
#define FB_ClientS_H

#include <Arduino.h>
#if defined(FB_ENABLE_EXTERNAL_CLIENT)
#include "./wcs/custom/FB_Custom_TCP_Client.h"
#define FB_TCP_CLIENT FB_Custom_TCP_Client
#elif defined(ESP32)
#include <WiFi.h>
#include "./wcs/esp32/FB_TCP_Client.h"
#define FB_TCP_CLIENT FB_TCP_Client
#elif defined(ESP8266)
#include <Schedule.h>
#include <ets_sys.h>
#include <ESP8266WiFi.h>
#include "./wcs/esp8266/FB_TCP_Client.h"
#define FB_TCP_CLIENT FB_TCP_Client
#define FS_NO_GLOBALS
#elif defined(PICO_RP2040)
#include <WiFi.h>
#include "./wcs/esp8266/FB_TCP_Client.h"
#define FB_TCP_CLIENT FB_TCP_Client
#else
#include "./wcs/custom/FB_Custom_TCP_Client.h"
#define FB_TCP_CLIENT FB_Custom_TCP_Client
#endif

#endif