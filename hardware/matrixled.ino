#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <FrekvensPanel.h>
#include <FS.h>

#define p_ena D6
#define p_data D5
#define p_clock D4
#define p_latch D3

ESP8266WebServer server(80);
FrekvensPanel panel(p_latch, p_clock, p_data);

int prg = 1;
String currentFile = "";

void setup() {

  Serial.begin(9600);
  Serial.println("Booted");


  // panel setup
  panel.clear();
  panel.scan();
  pinMode(p_ena, OUTPUT);
  analogWrite(p_ena, 0); // full brightness


  // wifi setup
  WiFi.begin("WLAN", "PASSWORD");

  Serial.print("Verbindung wird hergestellt ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Verbunden! IP Adresse: ");
  Serial.println(WiFi.localIP());

  // dns setup
  if (MDNS.begin("led")) {
    Serial.println("DNS gestartet!");
  }

  // server setup
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found!");
  });

  addServerRoutes();

  server.begin();

  // file setup
  SPIFFS.begin();
  if (!(SPIFFS.exists ("/data.csv") )) {
    File myfile = SPIFFS.open("/data.csv", "w");
    if (!myfile) {
      Serial.println("Error writing file");
    }
    myfile.close();
  } else {
    currentFile = getFileStr();
  }
}

void loop() {
  server.handleClient();

  switch (prg) {
    // random pixel mode
    case 1: {
      int x = random(16);
      int y = random(16);
      panel.drawPixel(x,y,random(6)==0);
      break;
    }

    // show animation coming from client
    case 2: {
      String str = currentFile;
      String strs[64];
      int StringCount = 0;
      while (str.length() > 0) {
        int index = str.indexOf("\n");
        if (index == -1) {
          strs[StringCount++] = str;
          break;
        } else {
          strs[StringCount++] = str.substring(0, index);
          str = str.substring(index+1);
        }
      }

      // show the resulting substrings
      for (int i = 0; i < StringCount; i++) {
        String animation = strs[i];

        String strs2[64];
        int StringCount2 = 0;
        while (animation.length() > 0) {
          int index = animation.indexOf(";");
          if (index == -1) {
            strs2[StringCount2++] = animation;
            break;
          } else {
            strs2[StringCount2++] = animation.substring(0, index);
            animation = animation.substring(index+1);
          }
        }

        for (int j = 0; j < StringCount2; j++) {

          if (j==0) {
            // ignore
          } else {
            renderStr(strs2[j]);
            panel.scan();
            delay(100);
          }
        }
      }

      break;
    }

    // none
    case 99: {
      panel.clear();
      break;
    }
  }

  panel.scan(); // refreshes display

  delay(10);
}

void addServerRoutes() {

  // api route
  server.on("/", HTTP_POST, []() {

    // check args
    if (!server.hasArg("text")) {
      server.send(500, "text/plain", "BAD ARGS");
      return;
    }

    String text = server.arg("text");

    server.sendHeader(F("Access-Control-Allow-Origin"),F("*"));
    server.send(200, "text/plain", "START");

    Serial.print("Start: ");
    Serial.println(text);

    renderStr(text);
  });

  // add row
  server.on("/set", HTTP_POST, []() {

    // check args
    DynamicJsonDocument doc(10240);

    if (!server.hasArg("plain")) {
      server.sendHeader(F("Access-Control-Allow-Origin"),F("*"));
      server.send(500, "text/plain", "BAD ARGS");
      return;
    }

    String message = server.arg("plain");
    DeserializationError error = deserializeJson(doc, message);

     if (error) {
      server.sendHeader(F("Access-Control-Allow-Origin"),F("*"));
      server.send(500, "text/plain", "Internal Server Error");
      return;
    }

    String text = doc["text"];

    // write contents into data csv
    File myfile = SPIFFS.open("/data.csv", "w");
    myfile.println(text);

    myfile.close();


    server.sendHeader(F("Access-Control-Allow-Origin"),F("*"));
    server.send(200, "application/json", "{\"file\":\"written\"}");
    currentFile = text;
  });

  // clear file
  server.on("/reset", []() {
    server.sendHeader(F("Access-Control-Allow-Origin"),F("*"));
    File myfile = SPIFFS.open("/data.csv", "w");

    if (!myfile) {
      server.send(200, "text/plain", "There was an error opening the file for clearing");
      return;
    }

    myfile.close();
    server.send(200, "text/plain", "File content has been cleared");
    currentFile = "";
  });

  // read file
  server.on("/read", []() {
    server.sendHeader(F("Access-Control-Allow-Origin"),F("*"));
    server.send(200, "text/plain", getFileStr());
  });

  // set program
  server.on("/setprg", HTTP_POST, []() {
    server.sendHeader(F("Access-Control-Allow-Origin"),F("*"));

    if (!server.hasArg("prg")) {
      server.send(500, "text/plain", "BAD ARGS");
      return;
    }

    String prg_str = server.arg("prg");

    prg = prg_str.toInt();

    server.send(200, "text/plain", "Set mode to: " + prg_str);

    // load file input
    if (prg == 2) {
      currentFile = getFileStr();
    }

    // reset panel
    panel.clear();
  });
}

String getFileStr() {
  File myfile = SPIFFS.open("/data.csv", "r");

  if (!myfile) {
    return "";
  }

  String fileContent = ""; // String to store the content of the file

  while (myfile.available()) {
    fileContent += char(myfile.read()); // Read and append each character to the string
  }

  return fileContent;
}

void renderStr(String text) {

  panel.clear();
  // render in matrix
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      char value = text.charAt(y * 16 + x);

      if (!value) break;

      panel.drawPixel(x,y,value=='1');
    }
  }
}
