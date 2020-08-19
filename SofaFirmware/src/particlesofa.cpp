#include "papertrail.h"
#include "mqtt.h"
#include "Seat.h"
#include "Button.h"
#include "Settings.h"
#include "publishqueue.h"
#include "ArduinoJson.h"
#include "DiagnosticsHelperRK.h"
#include "secrets.h"  // Contains variables such as API keys which should not be public

// Stubs
void seatCallback(int seat, int position);
void mqttCallback(char* topic, byte* payload, unsigned int length);

const int switchRelayPin = A6; // A6 on photon D13 on Argon
//const int switchRelayPin = D13; // A6 on photon D13 on Argon

Seat sofa[3] {
    {1, D3, D2, A0, A1, sofa, seatCallback},
    {2, D5, D4, A2, A3, sofa, seatCallback},
    {3, D7, D6, A4, A5, sofa, seatCallback}
};

// OTHER DETAILS
bool parentalMode;
bool measuringMode;
int modeValue = 0;
unsigned long lastRequestTime =  0;

uint32_t resetTime = 0;
retained uint32_t lastHardResetTime;
retained int resetCount;

PublishQueue pq;

PapertrailLogHandler papertrailHandler(papertrailAddress, papertrailPort,
  "Sofa", System.deviceID(),
  LOG_LEVEL_NONE, {
  { "app", LOG_LEVEL_ALL }
  // TOO MUCH!!! { “system”, LOG_LEVEL_ALL },
  // TOO MUCH!!! { “comm”, LOG_LEVEL_ALL }
});

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


    if (strstr(topic, "home/sofa/seat") != NULL) {
      int seat = topic[15] - '0';

      if (seat > 0 && seat <= 3) {
          if (strcmp(p, "stop") == 0) {
              sofa[seat-1].stopMoving();
          } else if (strcmp(p, "up") == 0) {
              sofa[seat-1].moveUp();
          } else if (strcmp(p, "down") == 0) {
              sofa[seat-1].moveDown();
          } else if (strcmp(p, "upright") == 0) {
              sofa[seat-1].moveToUpright();
          } else if (strcmp(p, "feetup") == 0) {
              sofa[seat-1].moveToFeet();
          } else if (strcmp(p, "flat") == 0) {
              sofa[seat-1].moveToFlat();
          }
      }
    } else if (strstr(topic, "home/sofa/parental_mode/set") != NULL) {
      if (strcmp(p, "enabled") == 0) {
        parentalMode = true;
      } else {
        parentalMode = false;
      }
      mqttClient.publish("home/sofa/parental_mode", parentalMode ? "enabled" : "disabled", true);
      digitalWrite(switchRelayPin, parentalMode);
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
        mqttClient.subscribe("home/sofa/parental_mode/set");
    } else {
        mqttConnectionAttempts++;
        Log.info("MQTT failed to connect");
    }
}

uint32_t nextMetricsUpdate = 0;
void sendTelegrafMetrics() {
    if (millis() > nextMetricsUpdate) {
        nextMetricsUpdate = millis() + 30000;

        char buffer[150];
        snprintf(buffer, sizeof(buffer),
            "status,device=Sofa uptime=%d,resetReason=%d,firmware=\"%s\",memTotal=%ld,memFree=%ld",
            System.uptime(),
            System.resetReason(),
            System.version().c_str(),
            DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_TOTAL_RAM),
            DiagnosticsHelper::getValue(DIAG_ID_SYSTEM_USED_RAM)
            );
        mqttClient.publish("telegraf/particle", buffer);
    }
}

int getSeatNumber(const char *input) {
  if (strlen(input) >= 1) {
    int returnValue = input[0] - '0';
    if (returnValue >= 0 && returnValue <= 3)
      return returnValue;
  }
  return -1;
}

void pressButton(int seat, int direction) {
  if (seat == 0) {
    for (int i = 0; i < 3; i++)
      sofa[i].startMoving(direction);
  }
  else
    sofa[seat-1].startMoving(direction);
}

void releaseButton(int seat) {
  if (seat == 0) {
    for (int i = 0; i < 3; i++)
      sofa[i].stopMoving();
  }
  else
    sofa[seat-1].stopMoving();
}

int moveTo(const char *data) {// Example input == "0,upright"
  int length = strlen(data)+1;
  char command[length];
  memcpy(command, data, length);
    
  const char *charSeat = strtok(command, ",");
  const char *action = strtok(NULL, ",");
  int seat = getSeatNumber(charSeat);

  if (strcmp(action, "upright") == 0) {
    if (seat == 0) {
      for (int i = 0; i < 3; i++)
        sofa[i].moveToUpright();
    } else {
      sofa[seat-1].moveToUpright();
    }
    return 0;
  } else if (strcmp(action, "feetup") == 0) {
    if (seat == 0) {
      for (int i = 0; i < 3; i++)
        sofa[i].moveToFeet();
    } else {
      sofa[seat-1].moveToFeet();
    }
    return 0;
  } else if (strcmp(action, "flat") == 0) {
    if (seat == 0) {
      for (int i = 0; i < 3; i++)
        sofa[i].moveToFlat();
    } else {
      sofa[seat-1].moveToFlat();
    }
    return 0;
  }
  return -1;
}

int moveManual(const char *data) { // Example input == "0,p,down" == on all seats press the down button
  int length = strlen(data)+1;
  char command[length];
  memcpy(command, data, length);

  const char *charSeat = strtok(command, ",");
  const char *charPress = strtok(NULL, ",");
  const char *charDirection;
  
  int seat = getSeatNumber(charSeat);
  bool press = charPress[0] == 'p';
  
  if (press) {
    charDirection = strtok(NULL, ",");
    int direction = charDirection[0] == 'u' ? UP : DOWN;
    pressButton(seat, direction);
  } else {
    releaseButton(seat);
  }
  return 0;
}

int saveSeatPosition(String command) {
  int seat = getSeatNumber(command.c_str());

  if (seat < 0  || seat > 3)
    return -1;

  int address; // 0 = feet up, 2 == flat

  if (command.substring(2,6).equals("feet"))
    address = 0;
  else if (command.substring(2,6).equals("flat"))
    address = 2;
  else
    return -1;

  if (seat == 0) {
    for (int i = 0; i < 3; i++)
      sofa[i].savePosition(address);
  }
  else
    sofa[seat-1].savePosition(address);

  return 0;
}

bool toggleMeasuringMode() {
  measuringMode = !measuringMode;
  for (int i = 0; i<3; i++)
    sofa[i].setMeasuringMode(measuringMode);
  return measuringMode;
}

int setMode(const char *command) {
  if (strcmp(command, "restart") == 0) {
    System.reset();
  } else if (strcmp(command, "measuring") == 0) {
    return toggleMeasuringMode();
  } else if (strstr(command, "measure") != NULL) { // command example "1,flat,measure" == measure flat position on seat 0
    return saveSeatPosition(String(command));
  } else
    return -1;

  return 0;
}

int getCombinedSofaPositions() {
  return (sofa[0].getCurrentPosition() + sofa[1].getCurrentPosition() + sofa[2].getCurrentPosition());
}

void random_seed_from_cloud(unsigned seed) {
  srand(seed);
}

SYSTEM_THREAD(ENABLED)

void startupMacro() {
    WiFi.selectAntenna(ANT_EXTERNAL);
    System.enableFeature(FEATURE_RESET_INFO);
    System.enableFeature(FEATURE_RETAINED_MEMORY);
}
STARTUP(startupMacro());

void setup() {
    waitFor(Particle.connected, 30000);
    
    do {
        resetTime = Time.now();
        Particle.process();
    } while (resetTime < 1500000000 || millis() < 10000);
    
    if (System.resetReason() == RESET_REASON_PANIC) {
        if ((Time.now() - lastHardResetTime) < 120) {
            resetCount++;
        } else {
            resetCount = 1;
        }

        lastHardResetTime = Time.now();

        if (resetCount > 3) {
            System.enterSafeMode();
        }
    } else if (System.resetReason() == RESET_REASON_WATCHDOG) {
      Log.info("RESET BY WATCHDOG");
    } else {
        resetCount = 0;
    }


  Particle.function("moveTo", moveTo);
  Particle.function("setMode", setMode);
  Particle.function("moveManual", moveManual);
  Particle.variable("getModeValue", modeValue);
  Particle.variable("resetTime", resetTime);

  Particle.publishVitals(900);

  pinMode(switchRelayPin, OUTPUT);
  digitalWrite(switchRelayPin, parentalMode);

  pq.publish("LOG", "Setup Complete");
}

void loop() {

  for (int i = 0; i < 3; i++)
    sofa[i].loop();

  if (mqttClient.isConnected()) {
    mqttClient.loop();
    sendTelegrafMetrics();
  } else if ((mqttConnectionAttempts < 5 && millis() > (lastMqttConnectAttempt + mqttConnectAtemptTimeout1)) ||
              millis() > (lastMqttConnectAttempt + mqttConnectAtemptTimeout2)) {
    connectToMQTT();
  }
    
  pq.process();
  wd.checkin(); // resets the AWDT count
}
