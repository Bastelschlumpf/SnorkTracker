/*
   Copyright (C) 2018 SFini

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
  * @file Mqtt.h
  *
  * Communication with an MQTT server.
  */


#include <PubSubClient.h>

#define topic_gsm_power              "SIM808/" MQTT_ID "/GsmPower"               //!< switch power on/off
#define topic_gsm_enabled            "SIM808/" MQTT_ID "/GsmEnabled"             //!< switch gsm on/off
#define topic_gps_enabled            "SIM808/" MQTT_ID "/GpsEnabled"             //!< switch gps on/off
#define topic_send_on_move_every     "SIM808/" MQTT_ID "/SendOnMoveEverySec"     //!< mqtt send interval on moving
#define topic_send_on_non_move_every "SIM808/" MQTT_ID "/SendOnNonMoveEverySec"  //!< mqtt sending interval on non moving
#define topic_msg                    "SIM808/" MQTT_ID "/Msg"                    //!< Last message
#define topic_cmd                    "SIM808/" MQTT_ID "/Cmd"                    //!< mqtt command for modul.

#define topic_voltage                "SIM808/" MQTT_ID "/Voltage"                //!< Power supply voltage

#define topic_temperature            "SIM808/" MQTT_ID "/BME280/temperature"     //!< Temperature
#define topic_humidity               "SIM808/" MQTT_ID "/BME280/humidity"        //!< Humidity
#define topic_pressure               "SIM808/" MQTT_ID "/BME280/pressure"        //!< Pressure

#define topic_imei                   "SIM808/" MQTT_ID "/Imei"                   //!< IMEI of the sim card
#define topic_cop                    "SIM808/" MQTT_ID "/Cop"                    //!< Operator selection
#define topic_csq                    "SIM808/" MQTT_ID "/Csq"                    //!< Signal quality
#define topic_batt_level             "SIM808/" MQTT_ID "/BattLevel"              //!< Batterie level on SIM808
#define topic_batt_volt              "SIM808/" MQTT_ID "/BattVolt"               //!< Batterie volt on SIM808
#define topic_gsm_loc                "SIM808/" MQTT_ID "/GsmLoc"                 //!< Location from the gsm message

#define topic_lon                    "SIM808/" MQTT_ID "/Gps/Longitude"          //!< Gps longitude
#define topic_lat                    "SIM808/" MQTT_ID "/Gps/Latitude"           //!< Gps latitude
#define topic_alt                    "SIM808/" MQTT_ID "/Gps/Altitude"           //!< Gps altitude
#define topic_kmph                   "SIM808/" MQTT_ID "/Gps/Kmh"                //!< Gps moving speed

/**
  * MQTT client for sending the collected data to a MQTT server
  */
class MyMqtt : protected PubSubClient 
{
protected:
   static MyOptions *g_myOptions;   //!< Static option pointer for the callback function.

public:
   static void mqttCallback(char* topic, byte* payload, unsigned int len);
   
protected:
   MyGsmGps  &myGsmGps;             //!< Reference to the Gsmgps instnces.
   MyOptions &myOptions;            //!< Reference to the options. 
   MyData    &myData;               //!< Reference to the data.

protected:
   void reconnect();
   bool sendData(); 

   using PubSubClient::connected;
   using PubSubClient::publish;

public:
   MyMqtt(MyGsmGps &gsmGps, MyOptions &options, MyData &data);
   ~MyMqtt();
   
   bool begin();
   void handleClient();
};

/* ******************************************** */

/** Constructor/Destructor */
MyMqtt::MyMqtt(MyGsmGps &gsmGps, MyOptions &options, MyData &data)
   : myGsmGps(gsmGps)
   , PubSubClient(gsmGps.gsmClient)
   , myOptions(options)
   , myData(data)
{
   g_myOptions = &options;
}
MyMqtt::~MyMqtt()
{
   g_myOptions = NULL;
}

/** Connects or reconnect to the MQTT server and subscribes the topics. */
void MyMqtt::reconnect()
{
   MyDbg("Attempting MQTT connection...");
   // Try 5 times to reconnected
   for (int i = 0; !PubSubClient::connected() && i < 5; i++) {
      // Attempt to connect
      if (PubSubClient::connect(MQTT_NAME, MQTT_USER, MQTT_PASSWORD)) {
         subscribe(topic_cmd);
         subscribe(topic_gsm_power);
         subscribe(topic_gsm_enabled);
         subscribe(topic_gps_enabled);
         subscribe(topic_send_on_move_every);
         subscribe(topic_send_on_non_move_every);
         MyDbg(" connected");
      } else {
         MyDbg(" Failed (" + String(i+1) + ") rc =" + String(state()));
         MyDbg(" Try again in 5 seconds");
         // Wait 5 seconds before retrying
         myDelay(5000);
         MyDbg(".", false, false);
      }
   }
}

/** Send the mqtt data if the gps values are new. */
bool MyMqtt::sendData() 
{
   MyDbg("Attempting MQTT publishing");
   if (PubSubClient::connected()) {
      publish(topic_voltage, String(myData.voltage).c_str(), true); 

      publish(topic_temperature, String(myData.temperature).c_str(), true); 
      publish(topic_humidity,    String(myData.humidity).c_str(),    true); 
      publish(topic_pressure,    String(myData.pressure).c_str(),    true); 
         
      publish(topic_csq,        myData.signalQuality.c_str(), true); 
      publish(topic_batt_level, myData.batteryLevel.c_str(),  true); 
      publish(topic_batt_volt,  myData.batteryVolt.c_str(),   true); 
         
      publish(topic_lon,  myData.longitude.c_str(), true); 
      publish(topic_lat,  myData.latitude.c_str(),  true); 
      publish(topic_alt,  myData.altitude.c_str(),  true); 
      publish(topic_kmph, myData.kmph.c_str(),      true); 
         
      MyDbg("mqtt published");
      return true;
   }
   return false;
}

/** Sets the MQTT server settings */
bool MyMqtt::begin()
{
   MyDbg("MQTT:begin");
   setServer(MQTT_SERVER, MQTT_PORT);
   setCallback(mqttCallback);

   return true;
}

/** Connect To the MQTT server and send the data when the time is right. */
void MyMqtt::handleClient()
{
   if (myGsmGps.isGsmActive) {
      if (!PubSubClient::connected()) {
         if (secondsElapsed(myData.rtcData.lastMqttReconnectSec, myOptions.mqttReconnectIntervalSec)) {
            reconnect();
         }
      }
      if (connected()) {
         bool send = false;

         if (myData.isMoving) {
            send = secondsElapsed(myData.rtcData.lastMqttSendSec, myOptions.mqttSendOnMoveEverySec);
         } else {
            send = secondsElapsed(myData.rtcData.lastMqttSendSec, myOptions.mqttSendOnNonMoveEverySec);
         }
         if (send) {
            sendData();
         }
      }
   }
}

MyOptions *MyMqtt::g_myOptions = NULL;

/** Static function for MQTT callback on registered topics. */
void MyMqtt::mqttCallback(char* topic, byte* payload, unsigned int len) 
{
   String strTopic = String((char*)topic);

   payload[len] = '\0';
   MyDbg("Message arrived [" + strTopic + "]:[ ");
   if (len) MyDbg((char *) payload);
   MyDbg("]");

   if (MyMqtt::g_myOptions) {
      if (strTopic == topic_gsm_power) {
         g_myOptions->gsmPower = atoi((char *) payload);
         MyDbg(strTopic + g_myOptions->gsmPower ? " - On" : " - Off");
      }
      if (strTopic == topic_gsm_enabled) {
         g_myOptions->isGsmEnabled = atoi((char *) payload);
         MyDbg(strTopic + g_myOptions->isGsmEnabled ? " - Enabled" : " - Disabled");
      }
      if (strTopic == topic_gps_enabled) {
         g_myOptions->isGpsEnabled = atoi((char *) payload);
         MyDbg(strTopic + g_myOptions->isGpsEnabled ? " - Enabled" : " - Disabled");
      }
      if (strTopic == topic_send_on_move_every) {
         g_myOptions->mqttSendOnMoveEverySec = atoi((char *) payload);
         MyDbg(strTopic + " - " + String(g_myOptions->mqttSendOnMoveEverySec));
      }
      if (strTopic == topic_send_on_non_move_every) {
         g_myOptions->mqttSendOnNonMoveEverySec = atoi((char *) payload);
         MyDbg(strTopic + " - " + String(g_myOptions->mqttSendOnNonMoveEverySec));
      }
   }
}
