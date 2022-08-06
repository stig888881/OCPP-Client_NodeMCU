// matth-x/ArduinoOcpp
// Copyright Matthias Akstaller 2019 - 2022
// MIT License

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;
#elif defined(ESP32)
#include <WiFi.h>
#else
#error only ESP32 or ESP8266 supported at the moment
#endif

#include <ArduinoOcpp.h>

#define STASSID "Keenetic-5742"
#define STAPSK  "88888888"

#define OCPP_HOST "192.168.1.36"
#define OCPP_PORT 9000
#define OCPP_URL "ws://192.168.1.36/"
#define SS_PIN 4  //D2
#define RST_PIN 5 //D1

#include <SPI.h>
#include <MFRC522.h>

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
int statuss = 0;
int out = 0;
int button = 15; //D8(gpio15)
int buttonState;
int change=0;
const char *idTag;
String FirstCard= "";
String SeconfCard= "";
//
////  Settings which worked for my SteVe instance
//
//#define OCPP_HOST "my.instance.com"
//#define OCPP_PORT 80
//#define OCPP_URL "ws://my.instance.com/steve/websocket/CentralSystemService/gpio-based-charger"
float Meter()
{
   float c=2235.34;
    return c;
}
void setup() {
    pinMode(button, INPUT);
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
    /*
     * Initialize Serial and WiFi
     */ 

    Serial.begin(115200);
    Serial.setDebugOutput(true);

    Serial.print(F("[main] Wait for WiFi: "));

#if defined(ESP8266)
    WiFiMulti.addAP(STASSID, STAPSK);
    while (WiFiMulti.run() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
#elif defined(ESP32)
    WiFi.begin(STASSID, STAPSK);
    while (!WiFi.isConnected()) {
        Serial.print('.');
        delay(1000);
    }
#else
#error only ESP32 or ESP8266 supported at the moment
#endif

    Serial.print(F(" connected!\n"));

    /*
     * Initialize the OCPP library
     */
    OCPP_initialize(OCPP_HOST, OCPP_PORT, OCPP_URL);

    /*
     * Integrate OCPP functionality. You can leave out the following part if your EVSE doesn't need it.
     */
    setPowerActiveImportSampler([]() {
        //measure the input power of the EVSE here and return the value in Watts
        return 0.f;
    });

    setOnChargingRateLimitChange([](float limit) {
        //set the SAE J1772 Control Pilot value here
        Serial.print(F("[main] Smart Charging allows maximum charge rate: "));
        Serial.println(limit);
    });

    setEvRequestsEnergySampler([]() {
        //return true if the EV is in state "Ready for charging" (see https://en.wikipedia.org/wiki/SAE_J1772#Control_Pilot)
        return false;
    });

    //... see ArduinoOcpp.h for more settings

    /*
     * Notify the Central System that this station is ready
     */
    bootNotification("My Charging Station", "My company name");
    
}


void loop() {
    buttonState=digitalRead(button);
    delay(10);
    /*
     * Do all OCPP stuff (process WebSocket input, send recorded meter values to Central System, etc.)
     */
    OCPP_loop();
    /*
     * Check internal OCPP state and bind EVSE hardware to it
     */
if (ocppPermitsCharge()) {
        //OCPP set up and transaction running. Energize the EV plug here
    } else {
        //No transaction running at the moment. De-energize EV plug
    }
    /*
     * Detect if something physical happened at your EVSE and trigger the corresponding OCPP messages
     */
    if ( mfrc522.PICC_IsNewCardPresent()) {
      mfrc522.PICC_ReadCardSerial();
       String content= "";
        //const char *idTag = "0123456789abcd"; //e.g. idTag = RFID.readIdTag();
         for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
    content.toUpperCase();
    FirstCard=content;
    idTag=strcpy(new char[FirstCard.length()+1],FirstCard.c_str());
        if (FirstCard!=SeconfCard){
        authorize(idTag, [] (JsonObject response) {
            //check if user with idTag is authorized
            if (!strcmp("Accepted", response["idTagInfo"]["status"] | "Invalid")){
                Serial.println(F("[main] User is authorized to start charging"));
            } else {
                Serial.printf("[main] Authorize denied. Reason: %s\n", response["idTagInfo"]["status"] | "");
            }
        });
        Serial.printf("[main] Authorizing user with idTag %s\n", idTag);
    }
    SeconfCard=FirstCard;
    }

    
    if (buttonState==1)
    {

     {//e.g. authorized idTag from above

        startTransaction(idTag, [] (JsonObject response) {
            //Callback: Central System has answered. Could flash a confirmation light here.
            Serial.printf("[main] Started OCPP transaction. Status: %s, transactionId: %u\n",
                    response["idTagInfo"]["status"] | "Invalid",
                    response["transactionId"] | -1);
        });
    }
    change=1;
    buttonState=0;
    }
    delay(10);
    buttonState=digitalRead(button);
    if (buttonState==1 && change==1) {
        //setEnergyActiveImportSampler(Meter);
        stopTransaction([] (JsonObject response) {
            //Callback: Central System has answered.
            Serial.print(F("[main] Stopped OCPP transaction\n"));
        });
        change=0;
    }

    //... see ArduinoOcpp.h for more possibilities
}