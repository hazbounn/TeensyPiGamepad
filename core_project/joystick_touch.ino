/*
Name:		joystick_touch.ino
Created:	5/6/2016 12:22:37 PM
Author:	Nicolas Hazboun
The following program allows a teensy 3.2 to behave as a gamepad and a resistive touchscreen diver simultaneously.
Documentation for implementing the touchscreen interface can be found here: [1] http://www.ti.com/lit/an/slaa384a/slaa384a.pdf


You are free to use this software for any purpose. You must however give credit to the author.
*/


#include <Bounce.h>

//      -----------Touch screen variables--------------
#define SCREEN_WIDTH 320.0
#define SCREEN_HEIGHT 240.0

//x+ and y+ are read from analog pins 0 and 1
enum { yPositiveAnalog = 1, xPositiveAnalog };		
//x+, y+, x- and y- are written to digital pins 14 to 17
enum { xNegative = 14, yPositiveDigital, xPositiveDigital, yNegative };

int xPos = 0;
int yPos = 0;
int prevXPos = 0;
int prevYPos = 0;
bool touchActive = true;
bool justPressed = false;


// ------------------gamepad variables---------------------------
//8 buttons will be used for the gamepad
static const uint8_t nButtons = 8;          
//associate each button with a pin number
enum buttons { y, b, a, x, l, r, st, sel, up, right, down, left };   


//button list declared as bounce entities in order to use the debounce function
Bounce buttonList[nButtons]{                       
	Bounce(y, 10),                          //10ms debounce to avoid misreads from mechanical tactile switches
	Bounce(b, 10),
	Bounce(a, 10),
	Bounce(x, 10),
	Bounce(l, 10),
	Bounce(r, 10),
	Bounce(st, 10),
	Bounce(sel, 10),
};

//4 buttons for the directional pad
Bounce dPad[4]{                               
	Bounce(up, 10),
	Bounce(right, 10),
	Bounce(down, 10),
	Bounce(left, 10),
};

//only allow two keys 0f the dpad to be pressed at the same time to beter manage hat angles.
float pressedDkeys[2] = { 0,0 };            
uint8_t npressedDkeys = 0;

//joystick axis reading values
int yValue; 
int xValue;


void setup() {
	pinMode(13, OUTPUT);
	digitalWriteFast(13, HIGH);
	
	//manually control when gamepad data is sent
	Joystick.useManualSend(true);             
	for (int i = 0; i<nButtons + 4; i++) {
		//pins configured as pullup resistors (signal is active low) so the switches can be connected to ground
		pinMode(i, INPUT_PULLUP);               
	}

	//touch screen setup
	pinMode(yNegative, INPUT);
	pinMode(xPositiveDigital, INPUT);
	pinMode(yPositiveDigital, INPUT_PULLUP);
	digitalWriteFast(xNegative, LOW);
	delay(100);
	attachInterrupt(yPositiveDigital, screenInterrupt, FALLING);
	Mouse.screenSize(SCREEN_WIDTH, SCREEN_HEIGHT);
}





void loop() {


	// ----------------------------------gamepad code--------------------------------

	for (uint8_t i = 1; i <= nButtons; i++)
	{
		//if there has been a change in the state of this button
		if (buttonList[i - 1].update())               
		{
			//if the button is being pressed
			if (buttonList[i - 1].fallingEdge())        
				Joystick.button(i, 1);                 //set the button as pressed
			else
				Joystick.button(i, 0);                 //else set it as released
		}
	}


	//button combinations:	l+r+a = start (map to enter key in retroarch); l+r+b = select (map to space key in retroarch); l+r+left = escape (useful in emulationStation)
	//						l+r+(up || down) = (volume up || down); 
	if (buttonList[l].read() == LOW && buttonList[r].read() == LOW)
	{
		if (buttonList[a].fallingEdge())
			Keyboard.set_key1(KEY_ENTER);
		else if (buttonList[a].risingEdge())
			Keyboard.set_key1(0);

		if (buttonList[b].fallingEdge())
			Keyboard.set_key1(KEY_SPACE);
		else if (buttonList[b].risingEdge())
			Keyboard.set_key1(0);
		
		if (dPad[3].fallingEdge())
			Keyboard.set_key1(KEY_ESC);
		else if (dPad[3].risingEdge())
			Keyboard.set_key1(0);

		if (dPad[2].fallingEdge()) 
			Keyboard.set_media(KEY_MEDIA_VOLUME_DEC);
		else if (dPad[2].risingEdge())
			Keyboard.set_media(0);
		
		if (dPad[0].fallingEdge()) 
			Keyboard.set_media(KEY_MEDIA_VOLUME_INC);
		else if (dPad[0].risingEdge())
			Keyboard.set_media(0);

	}else{
		//reset all buttons if not in "hotkey mode"
		Keyboard.set_key1(0);
		Keyboard.set_media(0);
	}

	//send keyboard data
	Keyboard.send_now();



	/*
	Logic behind the dPad:                
														  0
														-----
													   |     |
													   |     |
												 ------       ------
												|                   |
											 3  |                   |   1
												 ------       ------
													   |     |
													   |     |
														-----
														  2

	If two directions are pressed at the same time, we only want to combine adjacent directions which happens when the difference between button numbers is 1. e.g.: if we press buttons 2 and 1,
	2-1=1 so we can combine both directions and convert them to a 45 degree hat angle. If the difference between button numbers is not 1, we just send one of the directions (e.g: 0 and 2 are pressed and we send a 0 degree hat angle).
	One case is different however. If buttons 3 and 0 are pressed, 3-0=3 but it is still a valid combination that should be converted to a 315 degree hat angle.
	If only one direction is pressed, we send the equivalent angle.
	if no directions are pressed, the hat is brought back to a neutral position.
	*/

	//reset the number of pressed dpad directions
	npressedDkeys = 0;                       
	for (uint8_t i = 0; i < 4; i++)
	{
		dPad[i].update();
		//if the button is pressed and there are not already two buttons pressed
		if (!dPad[i].read() && npressedDkeys < 2)        
		{
			//add the direction to the key list 
			pressedDkeys[npressedDkeys] = i;               
			npressedDkeys++;
		}
	}

	switch (npressedDkeys)
	{
	case 0:
		Joystick.hat(-1);
		break;

	case 1:
		Joystick.hat(pressedDkeys[0] * 90.0);
		break;

	case 2:
		if (pressedDkeys[1] - pressedDkeys[0] == 1)
			Joystick.hat((pressedDkeys[1] + pressedDkeys[0]) / 2 * 90);
		else if (pressedDkeys[1] - pressedDkeys[0] == 3)
			Joystick.hat(315);
		else
			Joystick.hat(pressedDkeys[0]);
		break;
	}

	//map slider values to have full range on 3ds sliders.
	yValue = map(analogRead(4), 130, 850, 0, 1023);
	if (yValue > 530 || yValue < 447)
		Joystick.Y(yValue);
	else
		Joystick.Y(512);

	xValue = map(analogRead(5), 130, 850, 0, 1023);
	if (xValue > 530 || xValue < 465)
		Joystick.X(xValue);
	else
		Joystick.Y(512);



	//send gamepad data
	Joystick.send_now();                        



	/*
"The  x and  y coordinates  of  a touch  on  a 4-wire  touch  screen  can  be  read  in two  steps.  First,  Y+  is driven  high,
Y– is driven  to  ground,  and  the  voltage  at  X+  is measured.  The  ratio  of  this  measured  voltage  to  the  drive  voltage
applied  is equal  to  the  ratio  of  the  y coordinate  to  the  height  of  the  touch  screen.  The  y coordinate  can  be
calculated  as  shown  in Figure  3. The  x coordinate  can  be  similarly  obtained  by  driving  X+  high,  driving  X– to  ground,
and  measuring  the  voltage  at  Y+.  The  ratio  of  this  measured  voltage  to  the  drive  voltage  applied  is equal  to  the
ratio  of  the  x coordinate  to  the  width  of  the  touch  screen." [1]

													Y+
											 ------------------
											|					|
											|					|
											|					|
										X-	|					|  X+
											|					|
											|					|
											|					|
											 -------------------
													Y-

*/
	// --------------------------touch screen code----------------------------------------
	if (touchActive)
	{

		//set x- as an input to disconnect it from the circuit
		pinMode(xNegative, INPUT);											
		pinMode(xPositiveDigital, INPUT);
		pinMode(yNegative, OUTPUT);
		pinMode(yPositiveDigital, OUTPUT);
		// small delay to allow the pinmode to change (1 ms is the time necessary for the pin's capacitor to discharge)
		delay(1);
		//drive y- to 0 and y+ to 1
		digitalWriteFast(yNegative, LOW);
		digitalWriteFast(yPositiveDigital, HIGH);								
		//y = (Vx / vcc) * sreen_height
		yPos = ((analogRead(xPositiveAnalog) - 100) / (895.0 - 100)) * SCREEN_HEIGHT;		


		//same procedure for x axis
		pinMode(yNegative, INPUT);
		pinMode(yPositiveDigital, INPUT);
		pinMode(xNegative, OUTPUT);
		pinMode(xPositiveDigital, OUTPUT);
		digitalWriteFast(xNegative, LOW);
		digitalWriteFast(xPositiveDigital, HIGH);
		xPos = ((analogRead(yPositiveAnalog) - 80) / (937.0 - 80)) * SCREEN_WIDTH;

		




		//set y+ as pullup and check if the touch pannel is still active.
		pinMode(yNegative, INPUT);
		pinMode(xPositiveDigital, INPUT);
		pinMode(yPositiveDigital, INPUT_PULLUP);
		digitalWriteFast(xNegative, LOW);
		// small delay to allow the pinmode to change (1 ms is the time necessary for the pin's capacitor to discharge)
		delay(1);

		//if touch is no longer detected
		if (digitalReadFast(yPositiveDigital))
		{
			touchActive = false;
			Mouse.set_buttons(0, 0, 0);
			sei();
			attachInterrupt(yPositiveDigital, screenInterrupt, FALLING);
		}
		//only move by 10 pixel increments
		else if (abs(xPos - prevXPos) > 10 || abs(yPos - prevYPos) > 10)  
		{
			Mouse.moveTo(xPos, yPos);
			prevXPos = xPos;
			prevYPos = yPos;
		}



		if (justPressed) {
			Mouse.set_buttons(1, 0, 0);
			justPressed = false;
		}
		delay(20);

	}

}



void screenInterrupt()
{
	touchActive = true;
	justPressed = true;
	cli();
}












