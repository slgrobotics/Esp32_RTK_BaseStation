# Esp32 RTK BaseStation

## sends u-Blox ZED-F9P RTCM correction data to rtk2go.com NTRIP caster

This is heavily modified example NTRIP Server from SparkFun GNSS Library V2.0, u-Blox F9P section.

The idea is to:
   - let the station work hands-free forever
   - without USB connection to laptop
   - handle power outages, restore to normal operation
   - handle network and Caster glitches
   - avoid being banned by Caster for idle or numerous connections
   - provide ample visual indication about the station's status 

This code is free (as free beer) - no guarantees or liability of any kind. See MIT license below. Use at your own risk.
Send your suggestions to slg@quakemap.com  - beer accepted at https://www.paypal.com/paypalme/vitalbytes

Please support SparkFun.com - buy their awesome products! My pair of F9P breakboards works flawlessly and were easy to set up.

Please don't forget to set YOUR base station coordinates, see GNSS.ino tab around line 120
See "secrets.h" tab for credentials, set yours - for your WiFi network and the Caster

RTK2GO.com requires you to register as a base station data provider, it is easy and they respond within several hours.
 
Thank you for your interest,
-- Sergei Grichine
