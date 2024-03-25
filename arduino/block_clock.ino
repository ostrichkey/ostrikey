#include <Arduino.h>
#include "WiFiClientSecure.h"
#include "time.h"
#include <NostrEvent.h>
#include <NostrRelayManager.h>
#include <TFT_eSPI.h>
#include <vector>

const char* ssid     = "<SSID>"; // wifi SSID here
const char* password = "<PASS>"; // wifi password here

NostrEvent nostr;
NostrRelayManager nostrRelayManager;
NostrQueueProcessor nostrQueue;

bool hasSentEvent = false;

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

TFT_eSPI tft = TFT_eSPI(170, 320);

char const *pubkey = "480ec1a7516406090dc042ddf67780ef30f26f3a864e83b417c053a5a611c838";

int counter = 0;

void writeToDisplay(String text) {
    counter += 1;
    tft.setTextSize(2);
    //tft.fillScreen(TFT_BLACK);
    tft.setTextWrap(true, true);
    tft.drawString(text, 0, counter*20);
}

unsigned long getUnixTimestamp() {
  time_t now;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return 0;
  } else {
    Serial.println("Got timestamp of " + String(now));
  }
  time(&now);
  return now;
}

void nip01Event(const std::string& key, const char* payload) {
    Serial.println("NIP01 event");
    Serial.println("payload is: ");
    Serial.println(payload);
    writeToDisplay(payload);
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  Serial.println("hi nostr!");

  writeToDisplay("hi nostr!");
  writeToDisplay(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");

  Serial.println(WiFi.localIP());
  writeToDisplay(WiFi.localIP().toString());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  long timestamp = getUnixTimestamp();

  char mystr[40];
  sprintf(mystr, "%u", timestamp);
  writeToDisplay(mystr);

  std::vector<String> relays = {
    "relay.damus.io"
  };
    
  nostr.setLogging(true);
  nostrRelayManager.setRelays(relays);
  nostrRelayManager.setMinRelaysAndTimeout(2,10000);

  // Set some event specific callbacks here
  nostrRelayManager.setEventCallback(1, nip01Event);

  nostrRelayManager.connect();

  String subscriptionString = "[\"REQ\", \"" + nostrRelayManager.getNewSubscriptionId() + "\", {\"authors\": [\""+ pubkey +"\"], \"kinds\": [1], \"since\": "+(timestamp-50000)+"}]";
  nostrRelayManager.enqueueMessage(subscriptionString.c_str());
}

void loop() {
  nostrRelayManager.loop();
  nostrRelayManager.broadcastEvents();
}
