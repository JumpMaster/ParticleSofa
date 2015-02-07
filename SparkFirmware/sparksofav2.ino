#include "Seat.h"
#include "Button.h"

const int switchRelayPin = A6;
const unsigned long sleepTime = (60 * 60 * 1000); // 1 hour

Seat sofa[3];

// OTHER DETAILS
bool parentalMode;
bool crazyMode;
bool measuringMode;
int modeValue = 0;
unsigned long lastRequestTime =  0;
bool sleepCriteriaMet = false;

int getSeatNumber(String input)
{
  if (input.length() >= 1)
  {
    String strSeat = input.substring(0,1);

    if (isdigit(strSeat[0]))
      return strSeat[0] - '0';
    else
      return -1;
  }
  else
    return -1;
}

int moveTo(String command) // Example input == "0,upright"
{
  int seat = getSeatNumber(command);

  if (command.length() >= 6 && seat >= 0 && seat <= 3)
  {
    if (command.substring(2).equals("upright"))
    {
      if (seat == 0)
      {
        for (int i = 0; i < 3; i++)
          sofa[i].moveToUpright();
      }
      else
        sofa[seat-1].moveToUpright();

      logRequest();
      return true;
    }
    else if (command.substring(2).equals("feetup"))
    {
      if (seat == 0)
      {
        for (int i = 0; i < 3; i++)
          sofa[i].moveToFeet();
      }
      else
        sofa[seat-1].moveToFeet();

        logRequest();
        return true;
      }
      else if (command.substring(2).equals("flat"))
      {
        if (seat == 0)
        {
          for (int i = 0; i < 3; i++)
            sofa[i].moveToFlat();
        }
        else
          sofa[seat-1].moveToFlat();

        logRequest();
        return true;
      }
    }
    return -1;
}

int moveManual(String command) // Example input == "0,p,down" == on all seats press the down button
{
  if (command.length() >= 3)
  {
    int seat = getSeatNumber(command);
    if (seat >= 0  && seat <= 3)
    {
      if (command.substring(2, 3).equals("p"))
      {
        int direction;
        if (command.substring(4).equals("down"))
          direction = DOWN;
        else if (command.substring(4).equals("up"))
          direction = UP;
        else
          return -1;

        pressButton(seat, direction);
        return 0;
      }
      else if (command.substring(2, 3).equals("r"))
      {
        releaseButton(seat);
        return 0;
      }
    }
  }

  return -1;
}

void pressButton(int seat, int direction)
{
  if (seat == 0)
  {
    for (int i = 0; i < 3; i++)
      sofa[i].startMoving(direction);
  }
  else
    sofa[seat-1].startMoving(direction);
}

void releaseButton(int seat)
{
  if (seat == 0)
  {
    for (int i = 0; i < 3; i++)
      sofa[i].stopMoving();
  }
  else
    sofa[seat-1].stopMoving();
}

int setMode(String command)
{
  if (command.indexOf("parental") >= 0)
    return toggleParentalMode();
  else if (command.indexOf("crazy") >= 0)
    return toggleCrazyMode();
  else if (command.indexOf("restart") >= 0)
    System.reset();
  else if (command.indexOf("sleep") >= 0)
    lastRequestTime = millis() - sleepTime;
  else if (command.indexOf("measuring") >= 0)
    return toggleMeasuringMode();
  else if (command.indexOf("measure") >= 0) // command example "1,flat,measure" == measure flat position on seat 0
    return saveSeatPosition(command);
  else
    return -1;

  return 0;
}

bool toggleParentalMode()
{
  parentalMode = !parentalMode;
  digitalWrite(switchRelayPin, parentalMode);
  updateModeValue();
  return modeValue;
}

bool toggleCrazyMode()
{
  crazyMode = !crazyMode;
  setButtonPins();
  updateModeValue();
  return modeValue;
}

bool toggleMeasuringMode()
{
  measuringMode != measuringMode;
  for (int i = 0; i<3; i++)
    sofa[i].setMeasuringMode(measuringMode);
  return measuringMode;
}

void setButtonPins() {
  int pins[6] = {D2, D3, D4, D5, D6, D7};

  if (crazyMode)
  {
    randomSeed(millis());
    for (int a=0; a<6; a++)
    {
      int r = random(a, 6);
      int temp = pins[a];
      pins[a] = pins[r];
      pins[r] = temp;
    }
  }

  int a = 0;
  for (int i = 0; i < 3; i++)
    sofa[i].setButtonPins(pins[a++], pins[a++]);
}

void updateModeValue() {
  modeValue = parentalMode + (crazyMode * 2);
}

void logRequest() {
  lastRequestTime = millis();
}

int getCombinedSofaPositions() {
  return (sofa[0].getCurrentPosition() + sofa[1].getCurrentPosition() + sofa[2].getCurrentPosition());
}

int saveSeatPosition(String command)
{
  int seat = getSeatNumber(command);

  if (seat < 0  || seat > 3)
    return -1;

  int address; // 0 = feet up, 2 == flat

  if (command.substring(2,6).equals("feet"))
    address = 0;
  else if (command.substring(2,6).equals("flat"))
    address = 2;
  else
    return -1;

  if (seat == 0)
  {
    for (int i = 0; i < 3; i++)
      sofa[i].savePosition(address);
  }
  else
    sofa[seat-1].savePosition(address);

  return 0;
}

void setup() {
  Spark.function("moveTo", moveTo);
  Spark.function("setMode", setMode);
  Spark.function("moveManual", moveManual);

  Spark.variable("getModeValue", &modeValue, INT);

  for (int i = 0; i < 3; i++)
    sofa[i].setSeatNumber(i+1);

  setButtonPins();

  sofa[0].setRelayPins(A0, A1);
  sofa[1].setRelayPins(A2, A3);
  sofa[2].setRelayPins(A4, A5);

  sofa[0].loadPositions();
  sofa[1].loadPositions();
  sofa[2].loadPositions();

  delay(1000);
  /*sofa[0].setPositions(4900, 8000);*/
  /*sofa[1].setPositions(4900, 8000);*/
  /*sofa[2].setPositions(5300, 8300); // This sofa is slower then the other two*/

  pinMode(switchRelayPin, OUTPUT);
  digitalWrite(switchRelayPin, parentalMode);

  Spark.publish("LOG", "Setup Complete");
}

void loop() {
  if (Serial.available())
  {
    char inChar = (char)Serial.read();
    if (inChar == '\r')
    {
      Serial.println("1: "+String(sofa[0].getCurrentPosition()));
      Serial.println("2: "+String(sofa[1].getCurrentPosition()));
      Serial.println("3: "+String(sofa[2].getCurrentPosition()));
      Serial.println('\r');
    }
  }

  // check if anyone is requesting my services.
  if (sofa[0].run() || sofa[1].run() || sofa[2].run())
    logRequest();

  unsigned long timeSinceLastRequest = millis() - lastRequestTime;

  // We've been inactive for a while, no one is on the sofa,
  // and the buttons can wake me, let's warn people that I'll be turning off WiFi.
  if (!sleepCriteriaMet && getCombinedSofaPositions() == 0 && timeSinceLastRequest > sleepTime  && !parentalMode)
  {
    Spark.publish("LOG", "Sleeping in 30");
    sleepCriteriaMet = true;
  }
  else if (sleepCriteriaMet && timeSinceLastRequest > (sleepTime+30000) && WiFi.ready()) // That should be enough time for events to publish, sleep...
  {
    WiFi.off();
    /*Spark.sleep(D2,FALLING); // For future use on the Photon */
  }
  // We've been caught napping. When no one is looking turn WiFi back on.
  else if (timeSinceLastRequest < sleepTime && !WiFi.ready() && !WiFi.connecting())
  {
    // Spark.connect() briefly blocks code execution. Wait until the sofa isn't moving.
    if (!sofa[0].isMoving() && !sofa[1].isMoving() && !sofa[2].isMoving())
      Spark.connect();
  }
  // I'm awake and connected. Let everyone know.
  else if (sleepCriteriaMet && timeSinceLastRequest < sleepTime && Spark.connected())
  {
    sleepCriteriaMet = false;
    Spark.publish("LOG", "I'm awake!");
  }
}
