#include "papertrail.h"
#include "mqtt.h"
#include "Seat.h"
#include "Button.h"
#include "Settings.h"
#include "publishqueue.h"
#include "ArduinoJson.h"
#include "secrets.h"  // Contains variables such as API keys which should not be public

// Stubs
void seatCallback(int seat, int position);
void mqttCallback(char* topic, byte* payload, unsigned int length);

const int switchRelayPin = A6; // A6 on photon D13 on Argon
// const int switchRelayPin = D13; // A6 on photon D13 on Argon

Seat sofa[3] {
    {1, D3, D2, A0, A1, sofa, seatCallback},
    {2, D5, D4, A2, A3, sofa, seatCallback},
    {3, D7, D6, A4, A5, sofa, seatCallback}
    };

// OTHER DETAILS
bool parentalMode;
bool measuringMode;
int modeValue = 0;
unsigned long resetTime = 0;
unsigned long lastRequestTime =  0;

PublishQueue pq;

PapertrailLogHandler papertrailHandler(papertrailAddress, papertrailPort, "Sofa");

MQTT mqttClient(mqttServer, 1883, mqttCallback);
unsigned long lastMqttConnectAttempt;
const int mqttConnectAtemptTimeout1 = 5000;
const int mqttConnectAtemptTimeout2 = 30000;
unsigned int mqttConnectionAttempts;

ApplicationWatchdog wd(60000, System.reset);

// recieve message
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = '\0';

    int seat = topic[15] - '0';

    if (seat > 0 && seat <= 3) {
        if (strcmp(p, "stop") == 0)
            sofa[seat-1].stopMoving();
        else if (strcmp(p, "up") == 0)
            sofa[seat-1].moveUp();
        else if (strcmp(p, "down") == 0)
            sofa[seat-1].moveDown();
        else if (strcmp(p, "upright") == 0)
            sofa[seat-1].moveToUpright();
        else if (strcmp(p, "feetup") == 0)
            sofa[seat-1].moveToFeet();
        else if (strcmp(p, "flat") == 0)
            sofa[seat-1].moveToFlat();
    }
}

void seatCallback(int seat, int position) {
    if (mqttClient.isConnected()) {
        char topic[26];
        char cPosition[5];
        snprintf(cPosition, sizeof(cPosition), "%d", position);
        snprintf(topic, sizeof(topic), "home/sofa/seat/%d/position", seat);
        mqttClient.publish(topic, cPosition, true);
    }
}

void connectToMQTT() {
    lastMqttConnectAttempt = millis();
    bool mqttConnected = mqttClient.connect(System.deviceID(), mqttUsername, mqttPassword);
    if (mqttConnected) {
        mqttConnectionAttempts = 0;
        Log.info("MQTT Connected");
        mqttClient.subscribe("home/sofa/seat/+/set");
    } else {
        mqttConnectionAttempts++;
        Log.info("MQTT failed to connect");
    }
}

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

void updateModeValue() {
  modeValue = parentalMode;// + (crazyMode * 2);
}

bool toggleParentalMode()
{
  parentalMode = !parentalMode;
  digitalWrite(switchRelayPin, parentalMode);
  updateModeValue();
  return modeValue;
}

bool toggleMeasuringMode()
{
  measuringMode = !measuringMode;
  for (int i = 0; i<3; i++)
    sofa[i].setMeasuringMode(measuringMode);
  return measuringMode;
}

int setMode(String command)
{
  if (command.indexOf("parental") >= 0)
    return toggleParentalMode();
  else if (command.indexOf("restart") >= 0)
    System.reset();
  else if (command.indexOf("measuring") >= 0)
    return toggleMeasuringMode();
  else if (command.indexOf("measure") >= 0) // command example "1,flat,measure" == measure flat position on seat 0
    return saveSeatPosition(command);
  else
    return -1;

  return 0;
}

int getCombinedSofaPositions() {
  return (sofa[0].getCurrentPosition() + sofa[1].getCurrentPosition() + sofa[2].getCurrentPosition());
}

void random_seed_from_cloud(unsigned seed) {
   srand(seed);
}

SYSTEM_THREAD(ENABLED);

STARTUP(WiFi.selectAntenna(ANT_EXTERNAL)); // selects the u.FL antenna

void setup() {

  do
  {
    resetTime = Time.now();
    delay(10);
  } while (resetTime < 1000000 && millis() < 20000);

  Particle.function("moveTo", moveTo);
  Particle.function("setMode", setMode);
  Particle.function("moveManual", moveManual);
  Particle.variable("getModeValue", &modeValue, INT);
  Particle.variable("resetTime", &resetTime, INT);

  pinMode(switchRelayPin, OUTPUT);
  digitalWrite(switchRelayPin, parentalMode);

  pq.publish("LOG", "Setup Complete");
}

void loop() {

  for (int i = 0; i < 3; i++)
    sofa[i].run();

    if (mqttClient.isConnected()) {
        mqttClient.loop();
    } else if ((mqttConnectionAttempts < 5 && millis() > (lastMqttConnectAttempt + mqttConnectAtemptTimeout1)) ||
                 millis() > (lastMqttConnectAttempt + mqttConnectAtemptTimeout2)) {
        connectToMQTT();
    }
    
  pq.process();
  wd.checkin(); // resets the AWDT count
}
