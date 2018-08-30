# IFTTT_ESP8266
IFTTT_ESP8266 is a simple server+client project I've made to make my ESP8266 projects available and controllable by IFTTT. There are a lot of solutions in internet to make your projects reacheable from outside your LAN but all involve port forwarding (with the risks connected if you are not an experienced user and the possibility that your router doesn't permit it) and dynamicDNS services. This project overcomes these problems with a simple python server as intermediate.

## How it works
Here there is an example of interaction with Google Assistant and IFTTT:

![Schema](/images/IFTTT_interaction.svg)

## How to set it
### Server side
In "server" folder there is the simple python server running on Flask framework. Put it on your favourite hosting server (I've used pythonanywhere, it is free and there is a simple guide about how to configure it for Flask). Pay attention to also include the HTML files and to edit the WSGI file as illustrated on pythonanywhere. Go the URL provided by pythonanywhere for your app and click on Register; annotate the key the app will show you. Now create your own IFTTT applet or use [the one I've made](https://ifttt.com/applets/UrXQec9Z-ifttt_esp8266_ir) to control an IR device (pay attention to also edit the Webhooks fields).
### Client side
Everything you need is in "client" folder (except the external libraries). I've made a simple class to handle my [IRBlaster](https://www.thingiverse.com/thing:2756041). I'm planning to also create a class for an RGB led strip and set its transitions/color/state using IFTTT. The class is all commented and explained and I've also included an example file showing how to use it.

