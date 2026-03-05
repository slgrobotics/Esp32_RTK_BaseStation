# ESP32 RTK Base Station

## Sends u-blox ZED-F9P RTCM correction data to the rtk2go.com NTRIP caster.

This is a heavily modified example of an NTRIP Server from the SparkFun GNSS Library v2.0 (u-blox F9P section).
It presents itself as "*NTRIP SparkFun_RTK_Surveyor-SG*" on the "*agents*" page:
http://rtk2go.com:2101/SNIP::N-AGENT

The idea is to:
- Let the station run hands-free indefinitely
- Operate without a USB connection to a laptop
- Recover automatically after power outages
- Handle network and caster glitches
- Avoid being banned by the caster for idle or excessive connections
- Provide clear visual indication of the station's status

This code is free (as in free beer). No guarantees or liability of any kind. See the MIT license below. Use at your own risk.

Please support SparkFun.com — buy their awesome products! My pair of F9P breakout boards works flawlessly and was easy to set up.

The code compiles in the Arduino IDE with ESP32 support installed. My station uses a NodeMCU-32S.
Newer boards should also work, although the pinout may differ slightly.

The schematic is in the Media folder, along with photos of my very DIY unit. If you don't have NeoPixels — no worries; the LCD shows everything you need.

For general information on ESP32 GPIO pin usage, see this [video](https://youtu.be/E4LxQvyeiWw).

Please don't forget to set your base station coordinates. See the `GNSS.ino` tab around line 120.

See the `secrets.h` tab for credentials. Set your own values for your Wi-Fi network and the caster.

*RTK2GO.com* requires you to register as a base station data provider. The process is easy and they usually respond within a few hours.

Thank you for your interest.

-- Sergei Grichine

## Check these links:

- http://rtk2go.com - RTK Casters home page
- http://rtk2go.com:2101/SNIP::N-AGENT - agents list
- http://rtk2go.com:2101/SNIP::STATUS - current status
