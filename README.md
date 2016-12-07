# TeensyPiGamepad

Note: please view the raw Readme file for correct text formatting:
https://raw.githubusercontent.com/hazbounn/TeensyPiGamepad/master/README.md

Arduino code to use a Teensy 3.2 as a media controller for linux systems using 
Retroarch and EmualtionStation. The Teensy behaves as a gamepad, mouse and keyboard. 
This code provides basic gamepad funcionnality (for use with 3ds sliders as joysticks), 
a resistive touch screen driver which converts the user's touch input to usb mouse input, 
and volume control.

PIN MAPPINGS

Teensy pin 			Physical Component
0			y
1			b
2			a
3			x
4			l
5			r
6			start
7			select
8			dpad up
9			dpad right
10			dpad down 
11			dpad left
14 (analog 0)			touch x-
15 (analog 1)			touch y+
16 (analog 2)			touch x+
17 (analog 3)			touch y-
18 (analog 4)			slider Y
19 (analog 5)			slider X
	

FUNCTIONNALITY

Basic gamepad functionnality is implemented. You just need to map the buttons
in the retroarch configuration. Note that start button has been mapped to the
"enter" key and the select button has been maped to the "space" key (it may be 
easier to map them using a usb keyboard before using the Teensy). This
allowed easier navigation in Kodi and various dialog selections. 

A "hotkey" mode has also been implemented. It is activated by pressing the L and R buttons
at the same time. Hotkey functions: 
l+r+a = start (map to enter key in retroarch)
l+r+b = select (map to space key in retroarch) 
l+r+left = quit (mapped to the "escape" key. It is useful in EmulationStation and Retroarch )
l+r+(up || down) = (volume up || violume down)  (eliminates the need for hardware volume control)

EDITING AND BUILDING

Simply clone the repository and build the project. 
Any functionnality you do not want to use has to be commented out before compiling (especially the touchscreen code).
The project has been coded with Teensyduino 1.6 using the Visual Studio IDE with the Visual Micro plugin. It can be edited 
in the arduino IDE but comments will not appear the way they should because of Arduino's different text formatting. 
If you do not want to install arduino and want to keep all functionnalities (including the touch screen), you can 
simply flash the Teensy with the hex file found in the "./core_project/Debug" folder using the Teensy Loader application
downloaded from the PJRC website. 

Note: if you want to use regular analog joysticks instead of 3ds sliders:
change line 209 to: yValue = analogRead(4)
change line 215 to: xValue = analogRead(5)



Any improvements are welcome! Simply create a Pull Request and I will take a look. 
Send me a PM on bitbuilt.net/forums/ if you have any questions (username Pinotte).



