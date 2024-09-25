# TankSensor
ESP32=based sensor for tanks using 4-20mA sensor, and providing web access and graphics for readings


This project describes a system for measuring the fluid levels in a tank using a 4-20mA pressure sensor, and communicating that level to a human.
Other kinds of sensors could work as well, but they'd require some modifications to the code.

I'll refer to the sensing device as a "20mA sensor" for brevity. When you apply 24V to one of this sensor's wires, and 0V to the other, the sensor allows some current to flow: 4mA if the sensor is at the bottom of its range, 20mA if it's at the top (or higher) limit of its range. 

           Current-> 
+24V ------------------->SENSOR->.
 				 |
 				 |
0V <-----------------------------.

We will place a resistor in the circuit (we'll use 200 ohms in this example) like this

           Current-> 
+24V ------------------->SENSOR->.
 				 |
 				 |
0V <------[200 Ohm]--------------.

If 20mA flows through 200 Ohms, then by Ohm's Law (E = IR), we have that the voltage difference between the two ends of the resistor will be 
200 Ohms * (20/1000) A = 4000/1000 V =  4V. That's something we can measure with a voltmeter, for instance. 

Side note: the power dissipated by the resistor is I^2 R = (.002)^2 * 200 = .00004 * 200 = .008 W, i.e., much less than a tenth of a Watt, so a 1/8 watt resistor will suffice in this circuit. 

The sensor I'm using is a 2-meter depth sensor: when the water level is the same as the level of the sensor tip (usually the bottom of the tank), the current will be 4mA; when the water depth is 2 meters, it'll be 20mA. Of course, if my tank is only 0.8m, I'll never get to a reading of 20mA; the largest I'll ever see is (0.8m)/2m * 20 mA = 8mA. That'll give me a voltage difference of 8mA * 200 Ohm = 1.6V. So I'll need to make the software handle the fact that readings between 0.8V and 1.6V correspond to water depths in my tank between 0 (empty) and 0.8m (full). 

The first step is to get this voltage reading into the ESP32 processor. Fortunately, the ESP32 has an analog-to-digital converter (ADC). (It actually has two, but one of them is used by the WiFi software, so we have to use the other). For input voltages (relative to "ground") of 0 to 5V, the ADC pin produces readings of 0 to 4096. So our circuit will look like this:


+24V ------------------->SENSOR->.
 				 |
 				 |
0V <----.-[200 Ohm]---.----------.
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

We'll also need to supply power to the ESP32, and it's helpful to have at least one LED connected to give us some feedback. In this design, we're using an ESP32 that also has a TFT display unit attached to it (a "lilygo TTGO", https://www.banggood.com/LILYGO-TTGO-T-Display-ESP32-CH9102F-WiFi-bluetooth-Module-1_14-Inch-LCD-Development-Board-p-1999994.html?cur_warehouse=CN&ID=6309998&rmmds=search), and we'll use this to 
* transform sensor readings into useful numbers
* display a bar graph showing the fullness of the tank
* set lower and upper limits on the sensor readings 
* show the tank fullness via a web page that you can connect to at any time. 

Furthermore, there's also a web page that lets you adjust the name of that webpage, give the tank a name (perhaps "fuel tank 1", using another sensor for "fuel tank 2"), set the upper and lower limits of expected sensor readings, etc. And you can record Lower and upper limits using the buttons on the front of the device as well. Finally, the display is kind of bright, so there's a touch-sensor on the system: when you touch it with your finger, the display turns on for 10 seconds -- long enough for you to see how full the tank is -- and then turns off again.

When power is removed from the unit, the low- and high-limit settings, network name, tank name, and other data are preserved and restored at the next startup. For this, we use the Flash memory that's part of the ESP32, which can be written thousands of times before wearing out. Because our "settings" values are likely to be written just a few times, this seems like a safe approach. This particular part of the system is implemented in the Persistence.[cpp, h] files. 

The project involves several libraries:

* "Preferences", to handle our persistent data
* "Button2", to handle the limit-setting buttons
* "OTA", to handle over-the-air programming, so that the system can be updated via Bluetooth, hence without a physical reconnection to a laptop or other device with the Arduino programming environment
* "AsyncWebFS", a web-server that uses a file system implemented in the flash memory of the ESP32, and handles web data asynchronously
* mDNS, a system for providing DNS information (the stuff your computer uses to figure out that google.com really means IP address 27.82.18.1 [which I just made up]. This lets us give our device a network name like "fuelTank1.local" which a nearby device can connect to and read the current fullness of the fuel Tank
