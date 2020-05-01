// Compiles with Arduino IDE 1.6.5
#include <Bounce2.h>
#include <FiniteStateMachine.h>



State Off = State(enterOff, updateOff, exitOff); // burner is off

State Lighting = State(enterLighting, updateLighting, exitLighting); // burner is trying to light

State On = State(enterOn, updateOn, exitOn); // burner is on

State Alarm = State(updateAlarm);  //burner failed to light or went out

FSM Burner = FSM(Off); // start finite state machine in the Off state



#define GasRelayPin 12 //			3    instead using 12 y
#define IgniterRelayPin 11 //		2   instead using 11 y
#define CommandPin 4 //				4  y
#define StatusPin 5 //				5   y Power'd On not connected
#define OffStatusLedPin	9 //		9   y
#define LightingStatusLedPin 10 //	10   y
#define OnStatusLedPin 8 //		8   
#define FlameSensorPin	6 //		6     y
#define BuzzerPin	7 //			7  y

#define RelayOn		0//1  Logic reversal required by use of dual Relay module
#define RelayOff 	1//0

#define LedOn	1
#define LedOff	0

#define CommandOn	0
#define CommandOff	1
#define StatusOn	0
#define StatusOff	1

#define FlameOn		0
#define FlameOff	1

#define FlameWaitTime 	 	3000  // wait 2 sec for flame
#define GasClearTime		4000 // 10 sec for gas to dissipate
#define NumLightingTries 	3

#define BuzzerOn 0
#define BuzzerOff 1

int BurnerTries = 1; // counts tries to lightenterOff Burner
Bounce debouncer = Bounce(); // Instantiate a Bounce object

void setup()
{
	pinMode(GasRelayPin,OUTPUT);
	digitalWrite(GasRelayPin,RelayOff);
 
	pinMode(IgniterRelayPin,OUTPUT);
	digitalWrite(IgniterRelayPin,RelayOff);
	
	pinMode(CommandPin,INPUT_PULLUP); // off unless pulled low by brewery controller
	
	pinMode(StatusPin,OUTPUT);
	
	pinMode(OffStatusLedPin,OUTPUT);
	pinMode(LightingStatusLedPin,OUTPUT);
	pinMode(OnStatusLedPin,OUTPUT);
	//pinMode(FlameSensorPin,INPUT_PULLUP);
	
	pinMode(BuzzerPin,OUTPUT);
	digitalWrite(BuzzerPin,BuzzerOff);

 
  debouncer.attach(FlameSensorPin,INPUT_PULLUP); // Attach the debouncer to a pin with INPUT_PULLUP mode
  debouncer.interval(10); // Use a debounce interval of 10 milliseconds
	
} // end setup()


void loop()
	{
		Burner.update();
	} // end loop()


void enterOff()
{
	digitalWrite(StatusPin,StatusOff);
	digitalWrite(GasRelayPin,RelayOff);
	digitalWrite(IgniterRelayPin,RelayOff);
	
	digitalWrite(OffStatusLedPin,LedOn);
	digitalWrite(LightingStatusLedPin,LedOff);
	digitalWrite(OnStatusLedPin,LedOff);
} // end enterOff()

void updateOff()
{
	if(digitalRead(CommandPin) == CommandOn)
	Burner.transitionTo(Lighting);
} // end updateOff()

void exitOff()
	{
		BurnerTries = 1;
	}


void enterLighting()
	{
		digitalWrite(OffStatusLedPin,LedOff);
		digitalWrite(LightingStatusLedPin,LedOn);
		digitalWrite(OnStatusLedPin,LedOff);
	} // end enterLighting()

void updateLighting()
	{
			if(BurnerTries > NumLightingTries)
				{ // we are done trying!
					digitalWrite(GasRelayPin,RelayOff);
					digitalWrite(IgniterRelayPin,RelayOff);
					Burner.transitionTo(Alarm);
				}
					else // we are still trying!
						{
							BurnerTries = BurnerTries + 1;
							digitalWrite(IgniterRelayPin,RelayOn);  // try starting
             
							digitalWrite(GasRelayPin,RelayOn);
							unsigned long endtime = millis()+ FlameWaitTime;
              // Get the updated value :
              //int Flamestatus = debouncer.read();
							//while(digitalRead(FlameSensorPin) != FlameOn && millis()< endtime)
              debouncer.update();
              while(debouncer.read() != FlameOn && millis()< endtime)
								{
									if(digitalRead(CommandPin) == CommandOff)
										Burner.transitionTo(Off);
                  debouncer.update();
								} // end while
								
              // Get the updated value from FlameSensorPin :
              //int Flamestatus = debouncer.read();
              //if(digitalRead(FlameSensorPin) == FlameOn)
              debouncer.update();
							if(debouncer.read() == FlameOn)
								{
									digitalWrite(IgniterRelayPin,RelayOff);
									Burner.transitionTo(On);
								}
							//if(digitalRead(FlameSensorPin) == FlameOff)
              debouncer.update();
              if(debouncer.read() == FlameOff)
								{	// allow time for gas to clear before trying again ////////////////////////////////////
									unsigned long endtime = millis() + GasClearTime; 
									digitalWrite(GasRelayPin,RelayOff);
									digitalWrite(IgniterRelayPin,RelayOff);
									while(millis() < endtime)
										{
											if(digitalRead(CommandPin) == CommandOff)
												Burner.transitionTo(Off);
										} // end while(millis() < endtime)
								} // end if(digitalRead(FlameSensorPin) == FlameOff)
						} // end else of if(BurnerTries > NumLightingTries)
	} // end updateLighting()

void exitLighting()
	{
	
	}

void enterOn()
	{
		digitalWrite(StatusPin,StatusOn);
		digitalWrite(OnStatusLedPin,LedOn);
		digitalWrite(LightingStatusLedPin,LedOff);
		digitalWrite(OffStatusLedPin,LedOff);
	} // end enterOn()

void updateOn()
{
	// Get the updated value from FlameSensorPin :
  //int Flamestatus = debouncer.read();
	//if(digitalRead(FlameSensorPin) == FlameOff)
  debouncer.update();
	if(debouncer.read() == FlameOff)
	Burner.transitionTo(Alarm);
	if(digitalRead(CommandPin) == CommandOff)
	Burner.transitionTo(Off);
	
	digitalWrite(OnStatusLedPin,LedOff);
	delay(300);
	digitalWrite(OnStatusLedPin,LedOn);
	delay(300);
} // end updateOn()

void exitOn()
{
	
} // end exitOn()

void updateAlarm()
{ //we be beepin' and a flashin'
	digitalWrite(GasRelayPin,RelayOff);
	digitalWrite(IgniterRelayPin,RelayOff);
	
	digitalWrite(BuzzerPin,BuzzerOn);
	digitalWrite(OffStatusLedPin,LedOn);
	digitalWrite(LightingStatusLedPin,LedOn);
	digitalWrite(OnStatusLedPin,LedOn);
	delay(500);
	digitalWrite(BuzzerPin,BuzzerOff);
	digitalWrite(OffStatusLedPin,LedOff);
	digitalWrite(LightingStatusLedPin,LedOff);
	digitalWrite(OnStatusLedPin,LedOff);
	delay(500);
	
	// brewery controller must set Command to Off to cancel Alarm Mode
	if(digitalRead(CommandPin) == CommandOff)
	Burner.transitionTo(Off);
} //end updateAlarm()

