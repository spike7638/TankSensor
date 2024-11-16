# General notes

/* General notes 
This project will use a 4-20mA sensor to measure tank-fullness. There are several goals:
* Enter "empty" and "full" readings via the buttons on the EPS32 for initial setup
* In response to a touchpad touch, turn on the display and show a "level" gauge to give a visual sense of fullness
* Have that display turn off 19 seconds after the touuch is released
* Have the device connect to a local network with a default web page that shows how full the tank is
*
* ESP pinouts: https://sites.google.com/site/jmaathuis/arduino/lilygo-ttgo-t-display-esp32#h.p_uUP08r1njLqE
*
* The ESP-32 has two A AsyncFsWebServer units; the second one is used by the Wifi subsysten
* (see https://docs.espressif.com/projects/esp-idf/en/release-v4.4/esp32/api-reference/peripherals/adc.html)
* so we have to use the first (ADC1), which supports pins 32-39; we'll use pin 36 ("SVP" or "SENSOR_VP", where SVP and SVN are 
* names for a sensor measurement component that's now deprecated. Pin 36 is input only, with no pull-up or pull-down. 
*
* For touch-sensitive stuff,  we'll use pin 32, "TOUCH9" (which may be swapped with GPIO 33, depending on updates, so maybe we need to refer to 
* GPIO32 as T8 rather than T9)
*
* Temporarily, we'll use "touchPinWakeup" (pin 33) to signal that we should awake the display for 5 seconds, say. 
*
* For the low- and high-level setting feature, we'll use the left and right buttons, GPIO0 and GPIO35. 
*
* There's an LED temporarily attached to pin 2 to give feedback during development
*
* To avoid flicker, we'll use a sprite (see https://www.youtube.com/watch?v=sRGXQg7028A)
* 
* Also: OTA programming: https://lastminuteengineers.com/esp32-ota-updates-arduino-ide/
*/

# TankSensor
ESP32=based sensor for tanks using 4-20mA sensor, and providing web access and graphics for readings

---
cmd-K V : show markdown preview

[This is a link](www.google.com)

This is an image
![](https://cdn.britannica.com/39/233239-050-50C0C3C5/standard-poodle-dog.jpg?w=300)

quotes:
> "Four score and seven..."
> *Lincoln*

Lists: start line with dashes (or stars)
- One
- Two
        * two point 5
- Three

Numeric list: (note that my numbers don't matter!)/Users/jfh/Desktop/TankSensorProject/TankSensor/TODO.txt

1. item
        1. indented item
        1. indented two
1. item
code snippets: start with triple-backtick and then proglanguage name: C, cpp, python, ...
```cpp
public main(String s)
```
Skipping the language name gives raw text

Separators: use "---" with a blank line before

---
# TODO
.Make the "basic panel" show not only raw readings, but percentages relative to the lo and hi settings.  
* Add a meter display to the basic panel/Display
* Disable buttons unless you're in "power mode" (touch during startup)

Code cleanup, testing, and documentation
   . Add percentage display in web page
   .Neaten up .ino file to get rid of averaging stuff, etc.
   x Add “fuel gauge” display to web page

   x  Add option to decide which display to use; add to configuration panel
   Line 555 of Web.cpp: alternativeInit alone seems to not actually work, but using both sserver.init and alternativeInit seems possibly wrong. At least clean up comments
   In TankSensor.ino, I've turned off OTA to see whether the timeout still happens; it seems to fix the timeout problem. Hmm. 
   - Define “power mode”
      - “edit” and buttons and “setup” available in power mode

.Copy updated html files back to git. 

Testing
Startup with touch sensor off and bad wifi name; does portal appear when I go to esp-...? 
Does portal open and work? 
Startup with touch-sensor on and bad wifi name
Startup with good wifi, no touch-sensor
Is portal and edit hidden? 
Is DNS name working?
Is the sensor value display correct?
Startup with good wifi, touch-sensor on: am I in power mode? 
Does DNS name work? 
why does esp32-tank.local "die" after a few minutes?




# Overview
This project describes a system for measuring the fluid levels in a tank using a **4-20mA pressure sensor**, and communicating that level to a human. Other kinds of sensors could work as well, but they'd require some modifications to the code. We'll refer to this device and its software as a **monitor**. 

Here's an example of what the monitor display would show for a tank that's about 1/3 full, and whose critical fullness is indicated with a red line at about the 90% mark. 

![Sample Display](images/SampleDisplay.jpg)
Here's a web view of the data produced by this project:
![Sample Web](WebView.png)

(This is for a different tank, whose measurements range from 0 to 200 millimeters, so it's slightly more than 50% full). As you can see, the web view lets you also easily adjust various settings for the project.

This repository also includes a user manual, which describes the end-user use of an assembled monitor in an actual application, and ignores the details of the software, etc. 

There's a third kind of use, between the "build the device" person and the "how do I set it up and get readings?" person -- it's the person who wants to tinker with things a bit. For that, there's the "advanced use" section of the user manual. 

# Requirements
In its simplest use, the monitor needs only 12V power. This will allow you to calibrate it and use its display to see how full a tank is. But the intended use is that the device also be connected to a local-area network (LAN), which allows you to connect to it from, for example, a smartphone.

## Operation, broad description
To operate the system, you place the sensor at the bottom of a tank, and supply 12V power to the display head. Touching the touch-sensor in the device durns on the display for 10 seconds, so that you can see the measurement while not having the distracting light of the display at other times. There is also a Wifi connection available so that you cen read the sensor value that was as well. (Details below -- on first operation, some setup steps need to be done.)

To calibrate the device, there are two alternatives, each of which involves starting with an empty tank, and then filling it. In the simplest method, with the tank empty, you can press the left button on the device to establish the "empty" measurement. You then fill the tank, and press the right button to establish the "full" measurement. The "critical" line is set at 85% of the way between these two measurements. 

Alternatively, you can use the web interface, described below, to note the sensor readings when the tank is empty and full, and then (again through the web interface) enter these as the empty and full values, and choose your own critical value to use. Doing this kind of adjustment requires a password, which is displayed on the device during startup. (This is to prevent folks who can "see" the device on a wireless network from adjusting the settings without your knowledge.)

For advanced operation, including editing the appearance of the webpages used to access the system, physical access to the device is again required: if, during power-on of the device, you touch the touch-sensor of the device for 4 seconds, an additional webpage becomes accessible under the name `mysensor.local/edit`, where `mysensor` is the name you have chosen for your device. 

Finally, to enable over-the-air (OTA) programming of the ESP32 at the heart of this device, you can once again hold the touch-sensor at any time, which will make OTA programming available as long as the touch sensor is active.

## What's physically involved
The project involves a *sensor* and a *display unit* consisting of a TTGO ESP-with-TFT-display and two tiny power supplies, one providing 24V for the sensor, the other providing about 3V for the TTGO board. In my own implementation, each of these is powered from a 12V source, but there are other alternatives. The whole project uses no more than 1/10 Amp at 12V. 

I'll refer to the sensing device as a "20mA sensor" for brevity. It looks like this:
![Sensor](images/Sensor.jpg) 
and similar devices are [available from Amazon](https://www.amazon.com/dp/B07PXFPPMM/) and far cheaper from AliExpress. The red arrow points to the sensor head which needs to go at the bottom of the tank. The green arrow shows the two wires that connect to the sensor. The yellow arrow shows a "bulkhead gland" (purchased separately) that is screwed through a hole in the top of the tank, allowing the sensor cable to pass from the inside to the outside while not allowing liquid to pass. 

When you apply 24V to one of this sensor's wires (in our case, the red wire is marked "+"), and 0V to the other (in our case, the blue wire), the sensor allows some current to flow: 4mA if the sensor is at the bottom of its range, 20mA if it's at the top (or higher) limit of its range. 

```
           Current-> 
+24V ------------------->SENSOR->.
 				 |
 				 |
0V <-----------------------------.
```
We will place a resistor in the circuit (we'll use 150 ohms in this example) like this
```
           Current-> 
+24V ------------------->SENSOR->.
 				 |
 				 |
0V <------[150 Ohm]--------------.
```
If 20mA flows through 150 Ohms, then by Ohm's Law ($E = IR$), we have that the voltage difference between the two ends of the resistor will be 
150 Ohms * (20/1000) A = 3000/1000 V =  3V. That's something we can measure with a voltmeter, for instance, and it's also just below the threshold for the ADC pin on the ESP32, which can handle voltages up to 3.3V.  

Side note: the power dissipated by the resistor is $I^2 R = (.02)^2 * 150 = .06 W$, i.e., much less than a tenth of a Watt, so a 1/8 watt resistor will suffice in this circuit. 

The sensor I'm using is a 2-meter depth sensor: when the water level is the same as the level of the sensor tip (usually the bottom of the tank), the current will be 4mA; when the water depth is 2 meters, it'll be 20mA. Of course, if my tank is only 0.8m, I'll never get to a reading of 20mA; the largest I'll ever see is $(0.8m)/2m * 20 mA = 8mA$. That'll give me a voltage difference of $8mA * 150 Ohm = 1.2V$. So I'll need to make the software handle the fact that readings between 0.6V and 1.2V correspond to water depths in my tank between 0 (empty) and 0.8m (full). 

The first step is to get this voltage reading into the ESP32 processor. Fortunately, the ESP32 has an analog-to-digital converter (ADC). (It actually has two, but one of them is used by the WiFi software, so we have to use the other). For input voltages (relative to "ground") of 0 to 5V, the ADC pin (pin 32) produces readings of 0 to 4096. So our circuit will look like this:

```
+24V ------------------->SENSOR->.
 				 |
 				 |
0V <----.-[150 Ohm]---.----------.
        |             |
        |             |
        |             |
        |             |
        |             |
 __________________________________
|       G             A            |
|       N             D            |
|       D             C            |
|                                  |
|  ESP32                           |
|                                  |
 __________________________________
```
We'll also need to supply power to the ESP32, and it's helpful to have at least one LED connected to give us some feedback. The overall curcuit diagram looks like this, where each "Q" block is a DC-DC up-don buck booster power supply, available through EBay, AliExpress, etc. Where wires cross, there's no connection unless the crossing is a "*". I've omitted the LED, which (if you want it) should be connected to pin 2, through a 1K resistor, to ground. It's probably also wise to put a 1N4001 diode in series with the +12V line to prevent applying reversed voltage to the power supplies, which may melt if you do that. That diode is indicated with a "D" in the diagram. The end of the diode that's marked with a stripe is the end that goes to the 5V and 24V power supplies; the unstriped end goes to the +12V incoming voltage

```
               _____________
+12V -D--*--- |Vin+   Vout+|-----------------------------------------*
         |    |      Q     |                                         |
         |    |     5V     |                                         |
   0V ---|--*-| Vin-  Vout-|---------------------------------*       |
      |  |                                                |       |
         *--|--|Vin+   Vout+| ----------> Sensor --*         |       |
            |  |      Q     |                      |         |       |
            |  |    24V     |                      |         |       |
            *--| Vin-  Vout-|-*- [150 Ohm]---*-----*         |       |
                              |             |                |       |
                              |             |                |       |
                              |             |                |       |
                              |             |                |       |
                       __________________________________    |       |
                      |       G             A            |   |       |
                      |       N             D       GND  |---*       |
                      |       D             C            |           |
                      |                              +5  |-----------*
                      |  ESP32                           |
                      |                                  |
                       __________________________________
```
NB: These same pressure sensors are available in a form with three wires: 0, +5, and "sense", which ranges from 0 to +5. Using one of these, you can eliminae the second buck booster and the 150Ohm resistor, and power both the ESP32 
and the sensor from the same 5V buck-booster (or even a 7805 regulator!), simplifying things somewhat

In this design, we're using an ESP32 that also has a TFT display unit attached to it (a "lilygo TTGO", https://www.banggood.com/LILYGO-TTGO-T-Display-ESP32-CH9102F-WiFi-bluetooth-Module-1_14-Inch-LCD-Development-Board-p-1999994.html?cur_warehouse=CN&ID=6309998&rmmds=search), and we'll use this to 
* transform sensor readings into useful numbers
* display a bar graph showing the fullness of the tank
* set lower and upper limits on the sensor readings 
* show the tank fullness via a web page that you can connect to at any time. 

## Beyond hardware
As mentioned earlier, there are also a web pages for adjust various settings, like the network name for the project, the 'name' for the tank (perhaps "fuel tank 1")  set the upper and lower limits of expected sensor readings, etc. And you can record lower and upper limits using the buttons on the front of the device as well. Finally, the display is kind of bright, so there's a touch-sensor on the system: when you touch it with your finger, the display turns on for 10 seconds -- long enough for you to see how full the tank is -- and then turns off again. (The pin used for touch-sensing is pin 33.)

When power is removed from the unit, the low- and high-limit settings, network name, tank name, and other data are preserved and restored at the next startup. For this, we use the Flash memory that's part of the ESP32, which can be written thousands of times before wearing out. Because our "settings" values are likely to be written just a few times, this seems like a safe approach. This particular part of the system is implemented in the Persistence.[cpp, h] files. 

The project involves several libraries. A really useful "getting started" bunch of data is [here](https://sites.google.com/site/jmaathuis/arduino/lilygo-ttgo-t-display-esp32). Here are the libraries involved. 


* "Preferences", to handle our persistent data
* "ArduinoJson", to handle decoding data transferred from the web
* "Button2", to handle the limit-setting buttons
* "OTA", to handle over-the-air programming, so that the system can be updated over wireless, hence without a physical reconnection to a laptop or other device with the Arduino programming environment. OTA is only allowed when the touch-sensor is active (i.e., you have to hold it down to initiate and complete the update), providing a small level of security
* "AsyncWebFS", a web-server that uses a file system implemented in the flash memory of the ESP32, and handles web data asynchronously
* "ESPmDNS", a system for providing DNS information (the stuff your computer uses to figure out that google.com really means IP address 27.82.18.1 [which I just made up]. This lets us give our device a network name like "fuelTank1.local" which a nearby device can connect to and read the current fullness of the fuel Tank (this library is actually used by AsyncWebFS, but we also use it directly).
* "TFT_eSPI", a graphics library for the TFT display

These cooperate to provide the necessary functionality. For each, there are good web resources describing how they work, but for some (esp. the graphics library) there are some subtleties that I'll describe as well. 

The last of these -- TFT_eSPI -- requires selecting the board you're using **by editing the library file**; I cannot imagine what made the author make this choice. What if you have two projects that use two different boards? Presumably there's some magical innvocation you could use to make that work, but for now, the "edit the library" approach is the one we're taking. Specifically, you need to 

* Go to the root of the TFT_eSPI library. Most likely this will be C:\Users\<your name>\Documents\Arduino\libraries\TFT_eSPI

* Open the file User_Setup_select.h in a good text editor. For example Notepad++.

* Comment out the line `#include <User_Setup.h>`

* Uncomment the line, later in that file, that looks like this `#include <User_Setups/TTGO_T_Display.h>`

* Save the file.

It's probably not a bad idea to restart the IDE after this.

## Software structure
In ```TankSensor.ino```, in ```setup()```, we establish a few constants (e.g., how long to display the depth meter after a touch on the touch-sensor) and then initialize the various contributing components:
* `TouchAndSense` handles detection of a touch that turns on the display, and also getting a reading from the 20mA sensor
* ```Persistence``` handles the 'remembering' of settings like the sensor-name, the network-name, etc., from one power-on to the next. (It also handles re-setting these settings if you want to re-initialize everything and use the sensor in a different tank, etc.)
* ```Button``` handles the detection of button presses on the two buttons in the TTGO unit; the left one can be used to say "the current sensor reading should be used as the "lower limit" for this tank", while the right one similarly can set the upper limit. 
* ```OTA``` handles over-the-air reprogramming of the ESP32 device, useful for updating the device without having to uninstall it from the tank, etc. 
* ```Display``` handles drawing the bar-graph on the TTGO display, turning the display off and on, showing the system password on the display for the first few seconds of operation, etc. 
* ```Web``` handles all communications with the system via a web interface. That includes establishing a WiFi network, starting a DNS server that makes the device findable-by-name rather than by IP address, and handling callbacks for the various settings pages.

These are initialized in order, and then the main loop (```loop()```) runs repeatedly. Those familiar with the Arduino may be surprised to see nothing in the main loop checking for Wifi activity --- that's because the web server we're using is *asynchronous*, i.e., it somehow manages to do its thing without explicit calls from the main loop. 

The main loop itself checks to see whether the two buttons have been pressed, whether the device should accept new over-the-air programming, and then sends the current level-measurement to the display. Finally, it checks whether the touch-pad has been activated, and if so, turns on the display backlight until 10 seconds after the last touch-time. 

In the background, the web server is awaiting web requests and responding to them. Some of these requests are handled in the software itself; others by html documents in the webserver's filesystem. 

## Startup details
The expectation is that this monitor will be installed someplace with a local wireless network to which the device should attach itself. Let's say that this wireless network is called *BoatNet* with password *Sail*. 
When you first power up the sensor in a new environment, it'll try to connect to a network it's previously connected to (perhaps when you were testing it at home), or if this is the first time you've used it, it will be a name like `ESP-5A38EF2C`, starting with `ESP-` and followed by eight digits or letters. There's no password on this network. 

If it's either this first time, **or** the previous wifi network cannot be found, attempting to connect to the `ESP-...` network will lead you to a "captive portal" page where you can enter the correct credentials for your new environment. In our example case, we'd enter *BoatNet* as the "SSID", and *Sail* as the password. You want to be sure that these new credentials are saved so that you only have to do this once. To do so, make sure that the "slider" shown here is moved to the right, so that the credentials will be saved.
![Portal](images/CaptivePortal.png)
Then click on the  "Connect to SSID" button, and when prompted to reset the ESP connection, say "yes". 

At this point, any device connected to the BoatNet network should be able to connect to the sensor using the address `esp32-tank.local`. The first part of that -- `esp32-tank` is what's called the "network name" under the configuration settings. 

## Missing features
- I'd like to add broadcasting of the tank level via SignalK, so that various bits of sofware for boats could use it. 
- It'd be nice to have different options for the display of the tank level beyond the simple bar graph
- the OTA detection should probably be set up so that it's only checked during setup during a brief period -- perhaps 10 seconds -- so that we can ignore it in the main loop. 
- There's an "editor" that's part of the web server, and it should be turned off to prevent someone accidentally editing the various 'settings' webpages and screwing up the system. 
- It may be that the web server/editor combination can handle the OTA programming thing without the separate OTA module; if so, I'd like to delete the OTA portion of the code. 
