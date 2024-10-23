#include "Arduino.h"
#include "Persistence.h"

/*
 * record persistent data between power-ups. 
 * 
 * During setup, we check whether the stored password value exists; if not, we put in placeholder 
 * values for all of them. A "reset" simply removes the password value so that on the next reboot, 
 * everything will be initialized.
 *
 * Persistent data, on the ESP-32, comes in named groups. Ours is stored in a group called "persistentData". 
 * Within such a group, data is stored as key-value pairs, so depthSensorPreferences.putString("password", "Alberg37"); stores the string "Alberg37" 
 * with the key "password", for instance. 
 */
static Preferences depthSensorPreferences; 
extern String getDefaultWifiSSID();
extern String getDefaultWifiPassword();



void persistenceInit()
{
  depthSensorPreferences.begin("persistentData", false); // 'false" means "make this writeable"
  bool missing = !depthSensorPreferences.isKey("password");

  if (missing) {
    Serial.println("Initializing persistent data with default values");
    depthSensorPreferences.putString("password", "Alberg37");
    depthSensorPreferences.putString("networkName", "esp32-tanksensor");
    depthSensorPreferences.putString("sensorName", "TANK");
    depthSensorPreferences.putInt("lower_limit", 0);
    depthSensorPreferences.putInt("upper_limit", 100);
    depthSensorPreferences.putInt("critical_value", 85);
    depthSensorPreferences.putInt("critical_dir", 1); // Max key length is 15 characters...ugh!
    depthSensorPreferences.putString("WifiSSID", getDefaultWifiSSID());
    depthSensorPreferences.putString("WifiPassword", getDefaultWifiPassword());
    depthSensorPreferences.end();
  }
  else {
    Serial.println("Persistent data is present");
  }
}

// Used ONLY to reset data
void persistenceReset()
{
  depthSensorPreferences.begin("persistentData", false);
  depthSensorPreferences.remove("password");
  depthSensorPreferences.end();
  persistenceInit();
}

String getPassword()
{
  static String s;
  s = "";
  depthSensorPreferences.begin("persistentData", true); //"true" means "read only"
  s = depthSensorPreferences.getString("password");
  depthSensorPreferences.end();
  return s;
}

String getNetworkName()
{
  static String s;
  s = "";
  depthSensorPreferences.begin("persistentData", true);
  s = depthSensorPreferences.getString("networkName");
  depthSensorPreferences.end();
  return s;
}

String getSensorName()
{
  static String s;
  s = "";
  depthSensorPreferences.begin("persistentData", true);
  s = depthSensorPreferences.getString("sensorName");
  depthSensorPreferences.end();
  return s;
}

int getLowerLimit()
{
  int u; 
  depthSensorPreferences.begin("persistentData", true);
  u = depthSensorPreferences.getInt("lower_limit");
  depthSensorPreferences.end();
  return u;
}
int getUpperLimit()
{
  int u; 
  depthSensorPreferences.begin("persistentData", true);
  u = depthSensorPreferences.getInt("upper_limit");
  depthSensorPreferences.end();
  return u;
}

int getCriticalValue()
{
  int u; 
  depthSensorPreferences.begin("persistentData", true);
  u = depthSensorPreferences.getInt("critical_value");
  depthSensorPreferences.end();
  return u;
}

String getWifiSSID()
{
  static String s;
  s = "";
  depthSensorPreferences.begin("persistentData", true);
  s = depthSensorPreferences.getString("WifiSSID");
  depthSensorPreferences.end();
  return s;
}

String getWifiPassword()
{
  static String s;
  s = "";
  depthSensorPreferences.begin("persistentData", true);
  s = depthSensorPreferences.getString("WifiPassword");
  depthSensorPreferences.end();
  return s;
}

int getCriticalDirection()
{
  int u; 
  depthSensorPreferences.begin("persistentData", true);
  u = depthSensorPreferences.getInt("critical_dir");
  depthSensorPreferences.end();
  return u;
}

void setPassword(String s)
{
  depthSensorPreferences.begin("persistentData", false);
  depthSensorPreferences.putString("password", s);
  depthSensorPreferences.end();
}

void setNetworkName(String s)
{
  depthSensorPreferences.begin("persistentData", false);
  depthSensorPreferences.putString("networkName", s);
  depthSensorPreferences.end();
}
void setSensorName(String s)
{
  depthSensorPreferences.begin("persistentData", false);
  depthSensorPreferences.putString("sensorName", s);
  depthSensorPreferences.end();
}

void setLowerLimit(int lim)
{
  depthSensorPreferences.begin("persistentData", false);
  depthSensorPreferences.putInt("lower_limit", lim);
  depthSensorPreferences.end();
}
void setUpperLimit(int lim)
{
  depthSensorPreferences.begin("persistentData", false);
  depthSensorPreferences.putInt("upper_limit", lim);
  depthSensorPreferences.end();
}
void setCriticalValue(int val)
{
  depthSensorPreferences.begin("persistentData", false);
  depthSensorPreferences.putInt("critical_value", val);
  depthSensorPreferences.end();
}
void setCriticalDirection(int val)
{
  depthSensorPreferences.begin("persistentData", false);
  depthSensorPreferences.putInt("critical_dir", val);
  depthSensorPreferences.end();
}

void setWifiPassword(String s)
{
  depthSensorPreferences.begin("persistentData", false);
  depthSensorPreferences.putString("WifiPassword", s);
  depthSensorPreferences.end();
}