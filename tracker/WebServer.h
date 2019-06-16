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
  * @file WebServer.h
  *
  * Webserver Interface to communicate via esp8266 in station or access-point mode. 
  * Works with the SPIFFS (.html, .css, .js).
  */


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include "Spiffs.h"
#include "HtmlTag.h"

/**
  * MyESPWebServer helper class for accessing the internal _currentClient.
  */
class MyESPWebServer : public ESP8266WebServer
{
public:
   MyESPWebServer(int port = 80)
      : ESP8266WebServer(port) { }

   WiFiClient &wifiClient() { return _currentClient; }
};

/**
  * My Webserver interface. Works together with .html, .css and .js files from the SPIFFS.
  * Works mostly with static functions because of the server callback functions.
  */
class MyWebServer
{
public:
   static MyESPWebServer server;    //!< Webserver helper class.
   static IPAddress      ip;        //!< Soft AP ip Address
   static DNSServer      dnsServer; //!< Dns server
   static MyOptions     *myOptions; //!< Reference to the options.
   static MyData        *myData;    //!< Reference to the data.

protected:
   static bool loadFromSpiffs  (String path);
   static void AddTableBegin   (String &info);
   static void AddTableTr      (String &info);
   static void AddTableTr      (String &info, String name, String value);
   static void AddTableEnd     (String &info);
   static bool GetOption       (String id, String &option);
   static bool GetOption       (String id, long   &option);
   static bool GetOption       (String id, double &option);
   static bool GetOption       (String id, bool   &option);
   static void AddBr           (String &info);
   static void AddOption       (String &info, String id, String name, bool value, bool addBr = true);
   static void AddOption       (String &info, String id, String name, String value, bool addBr = true, bool isPassword = false);
   static void AddIntervalInfo (String &info);

public:
   static void handleRoot();
   static void loadMain();
   static void handleLoadMainInfo();
   static void loadUpdate();
   static void loadSettings();
   static void handleLoadSettingsInfo();
   static void handleSaveSettings();
   static void handleLoadInfoInfo();
   static void loadConsole();
   static void handleLoadConsoleInfo();
   static void loadRestart();
   static void handleLoadRestartInfo();
   static void handleNotFound();
   static void handleWebRequests();

public:
   bool isWebServerActive; //!< Is the webserver currently active.

public:
   MyWebServer(MyOptions &options, MyData &data);
   ~MyWebServer();

   bool begin();
   void handleClient();
};

/* ******************************************** */

IPAddress      MyWebServer::ip(192, 168, 1, 1);       
DNSServer      MyWebServer::dnsServer;
MyESPWebServer MyWebServer::server(80);
MyOptions     *MyWebServer::myOptions = NULL;
MyData        *MyWebServer::myData    = NULL;


/** Constructor/Destructor */
MyWebServer::MyWebServer(MyOptions &options, MyData &data)
   : isWebServerActive(false)
{
   myOptions = &options;
   myData    = &data;      
}
MyWebServer::~MyWebServer()
{
   myOptions = NULL;
   myData    = NULL;      
}

/** Starts the Webserver in station and/or ap mode and sets all the callback functions for the specific urls. 
  * If you use rtc yourself you have to switch off the automatic Wifi configuration with WiFi.persistent(false)
  * (because it uses also the RTC memory) otherwise the WIFI won't start after a Deep Sleep.
  */
bool MyWebServer::begin()
{
   if (!myOptions || !myData) {
      return false;
   }

   MyDbg(F("MyWebServer::begin"));
   WiFi.persistent(false);
   WiFi.mode(WIFI_AP_STA);
   WiFi.softAP(SOFT_AP_NAME, SOFT_AP_PW);
   WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));  
   dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
   dnsServer.start(53, F("*"), ip);
   myData->softAPIP         = WiFi.softAPIP().toString();
   myData->softAPmacAddress = WiFi.softAPmacAddress();
   MyDbg((String) F("SoftAPIP address: ")     + myData->softAPIP, true);
   MyDbg((String) F("SoftAPIP mac address: ") + myData->softAPmacAddress, true);

   if (myOptions->connectWifiAP) {
      WiFi.begin(myOptions->wifiAP.c_str(), myOptions->wifiPassword.c_str());
      for (int i = 0; i < 30 && WiFi.status() != WL_CONNECTED; i++) { // 30 Sec versuchen
         MyDbg(F("."), true, false);
         MyDelay(1000);
      }
   }
   if (WiFi.status() == WL_CONNECTED) {
      myData->stationIP = WiFi.localIP().toString();
      MyDbg((String) F("Connected to ")        + myOptions->wifiAP, true);
      MyDbg((String) F("Station IP address: ") + myData->stationIP, true);
      MyDbg((String) F("AP1 SSID (RSSI): ")    + String(myOptions->wifiAP + F(" (") + WifiGetRssiAsQuality(WiFi.RSSI()) + F("%)")));
   } else { // switch to AP Mode only
      if (myOptions->connectWifiAP) {
         MyDbg((String) F("No connection to ") + myOptions->wifiAP, true);
      }
      WiFi.disconnect();
      WiFi.mode(WIFI_AP);
   }
   
   server.on(F("/"),              handleRoot);
   server.on(F("/Main.html"),     loadMain);
   server.on(F("/MainInfo"),      handleLoadMainInfo);
   server.on(F("/Update.html"),   loadUpdate);
   server.on(F("/Settings.html"), loadSettings);
   server.on(F("/SettingsInfo"),  handleLoadSettingsInfo);
   server.on(F("/SaveSettings"),  handleSaveSettings);
   server.on(F("/InfoInfo"),      handleLoadInfoInfo);
   server.on(F("/Console.html"),  loadConsole);
   server.on(F("/ConsoleInfo"),   handleLoadConsoleInfo);
   server.on(F("/Restart.html"),  loadRestart);
   server.on(F("/RestartInfo"),   handleLoadRestartInfo);
   server.onNotFound(handleWebRequests);

   server.begin(); 
   MyDbg(F("Server listening"), true);

   isWebServerActive = true;
   return true;
}

/** Handle the http requests. */
void MyWebServer::handleClient()
{
   if (isWebServerActive) {
      server.handleClient();
      dnsServer.processNextRequest();  
   }
}

/** Helper function to start a HTML table. */
void MyWebServer::AddTableBegin(String &info)
{
   info += F("<table style='width:100%'>");
}

/** Helper function to write one HTML row with no data. */
void MyWebServer::AddTableTr(String &info)
{
   info += F("<tr><th></th><td>&nbsp;</td></tr>");
}

/** Helper function to add one HTML table row line with data. */
void MyWebServer::AddTableTr(String &info, String name, String value)
{
   if (value != "") {
      info += F("<tr>");
      info += (String) F("<th>") + TextToXml(name)  + F("</th>");
      info += (String) F("<td>") + TextToXml(value) + F("</td>");
      info += F("</tr>");
   }
}
  
/** Helper function to add one HTML table end element. */
void MyWebServer::AddTableEnd(String &info)
{
   info += F("</table>");
}

/** Reads one string option from the url args. */
bool MyWebServer::GetOption(String id, String &option)
{
   if (server.hasArg(id)) {
      option = server.arg(id);
      return true;
   }
   return false;
}

/** Reads one long option from the URL args. */
bool MyWebServer::GetOption(String id, long &option)
{
   bool   ret = false;
   String opt = server.arg(id);

   if (opt != "") {
      if (opt.indexOf(F(":")) != -1) {
         ret = scanInterval(opt, option);
      }  else {
         option = atoi(opt.c_str());
         ret    = true;
      }
   }
   return ret;
}

/** Reads one double option from the URL args. */
bool MyWebServer::GetOption(String id, double &option)
{
   String opt = server.arg(id);

   if (opt != "") {
      option = atof(opt.c_str());
      return true;
   }
   return false;
}

/** Reads one bool option from the URL args. */
bool MyWebServer::GetOption(String id, bool &option)
{
   String opt = server.arg(id);

   option = false;
   if (opt != "") {
      if (opt == F("on")) {
         option = true;
      }
      return true;
   }
   return false;
}

/** Add a HTML br element. */
void MyWebServer::AddBr(String &info)
{
   info += F("<br />");
}

/** Add one string input option field to the HTML source. */
void MyWebServer::AddOption(String &info, String id, String name, String value, bool addBr /* = true */, bool isPassword /* = false */)
{
   info += (String) F("<b>") + TextToXml(name) + F("</b>");
   info += (String) F("<input id='") + id + F("' name='") + id + F("' ");
   /*
    * Autocomplete overwrites our password if the type is password :(
    * So we show it actually in clear text.
   if (isPassword) {
      info += " type='password' autocomplete='new-password' ";
   }
   */
   info += (String) F("value='") + TextToXml(value) + F("'>");
   if (addBr) {
      info += F("<br />");
   }
}

/** Add one bool input option field to the HTML source. */
void MyWebServer::AddOption(String &info, String id, String name, bool value, bool addBr /* = true */)
{
   info += F("<input style='width:auto;' id='");
   info += id;
   info += F("' name='");
   info += id;
   info += F("' type='checkbox' ");
   info += value ? F(" checked") : F("");
   info += F("><b>");
   info += TextToXml(name);
   info += F("</b>");
   if (addBr) {
      info += F("<br />");
   }
}

/** Add the format information of the intervall in 'dd hh:mm:ss' */
void MyWebServer::AddIntervalInfo(String &info)
{
   info += (String) F("<p>") + TextToXml(F("Interval in '[days] hours:minutes:seconds' or just 'seconds'")) + F("</p>");
   info += F("<br />");
}

/** Helper function to load a file from the SPIFFS. */
bool MyWebServer::loadFromSpiffs(String path)
{
   bool ret = false;
   
   String dataType = F("text/plain");
   if(path.endsWith("/")) path += F("index.htm");
   
   if(path.endsWith(F(".src"))) path = path.substring(0, path.lastIndexOf("."));
   else if(path.endsWith(F(".html"))) dataType = F("text/html");
   else if(path.endsWith(F(".htm")))  dataType = F("text/html");
   else if(path.endsWith(F(".css")))  dataType = F("text/css");
   else if(path.endsWith(F(".js")))   dataType = F("application/javascript");
   else if(path.endsWith(F(".png")))  dataType = F("image/png");
   else if(path.endsWith(F(".gif")))  dataType = F("image/gif");
   else if(path.endsWith(F(".jpg")))  dataType = F("image/jpeg");
   else if(path.endsWith(F(".ico")))  dataType = F("image/x-icon");
   else if(path.endsWith(F(".xml")))  dataType = F("text/xml");
   else if(path.endsWith(F(".pdf")))  dataType = F("application/pdf");
   else if(path.endsWith(F(".zip")))  dataType = F("application/zip");
   
   File dataFile = SPIFFS.open(path.c_str(), "r");
   if (dataFile) {
      if (server.hasArg(F("download"))) {
         dataType = F("application/octet-stream");
      }
      if (server.streamFile(dataFile, dataType) == dataFile.size()) {
         ret = true;
      }
      dataFile.close();
   }
   return ret;
}

/** Redirect a root call to the Main.html site. */
void MyWebServer::handleRoot()
{
   server.sendHeader(F("Location"), F("Main.html"), true);
   server.send(302, F("text/plain"), "");
}

/** Sends the Main.html to the client. */
void MyWebServer::loadMain()
{
   if (loadFromSpiffs(F("/Main.html"))) {
      return;
   }
   handleNotFound();
}

/** Handle the ajax call of the Main.html detail information. */
void MyWebServer::handleLoadMainInfo()
{
   if (!myOptions || !myData) {
      return;
   }
   
   String info;
   String onOff = server.arg(F("o"));

   if (onOff == F("1")) {
      myOptions->powerOn = !myOptions->powerOn;
      myOptions->save();
   }

   AddTableBegin(info);
   if (myData->status != "") {
      AddTableTr(info, F("Status"), myData->status);
   }
#ifdef SIM808_CONNECTED
   AddTableTr(info, F("Modem Info"),  myData->modemInfo);
#else
   if (WiFi.status() != WL_CONNECTED) {
      AddTableTr(info, F("AP SSID"), String(SOFT_AP_NAME));
   } else {
      AddTableTr(info, F("AP SSID (RSSI)"), String(myOptions->wifiAP + F(" (") + WifiGetRssiAsQuality(WiFi.RSSI()) + F("%)")));
   }
   AddTableTr(info);
#endif   

   AddTableTr(info, F("Battery"),         String(myData->voltage,     2) + F(" V"));
   AddTableTr(info, F("Temperature"),     String(myData->temperature, 1) + F(" °C"));
   AddTableTr(info, F("Humidity"),        String(myData->humidity,    1) + F(" %"));
   AddTableTr(info, F("Pressure"),        String(myData->pressure,    1) + F(" hPa"));
#ifndef SIM808_CONNECTED
   AddTableTr(info, F("Active Time"),     formatInterval(myData->getActiveTimeSec()));
   AddTableTr(info, F("PowerUpTime"),     formatInterval(myData->getPowerOnTimeSec()));
   AddTableTr(info, F("DeepSleepTime"),   formatInterval(myData->rtcData.deepSleepTimeSec));
   AddTableTr(info, F("mAh"),             String(myData->getPowerConsumption(), 2));
   AddTableTr(info, F("Low power mAh"),   String(myData->getLowPowerPowerConsumption(), 2));
#endif   
   if (myData->rtcData.lastGps.fixStatus) {
      AddTableTr(info, F("Longitude"),  myData->rtcData.lastGps.longitudeString());
      AddTableTr(info, F("Latitude"),   myData->rtcData.lastGps.latitudeString());
      AddTableTr(info, F("Altitude"),   myData->rtcData.lastGps.altitudeString() + F(" m"));
      AddTableTr(info, F("Speed"),      myData->rtcData.lastGps.kmphString()     + F(" kmph"));
      AddTableTr(info, F("Satellites"), myData->rtcData.lastGps.satellitesString());
   }

   if (myOptions->isMqttEnabled) {
      MyTime mqttLastSentTime(myData->rtcData.mqttLastSentTime);

      AddTableTr(info, F("MQTT sent"), String(myData->rtcData.mqttSendCount));
#ifdef SIM808_CONNECTED
      AddTableTr(info, F("MQTT last"), mqttLastSentTime.timeString());
#endif
   }

   if (myData->secondsToDeepSleep >= 0) {
      AddTableTr(info, F("Power saving in "), String(myData->secondsToDeepSleep) + F(" Seconds"));
   }
   AddTableEnd(info);
   
   info += (String) 
      F("<table style='width:100%'>"
         "<tr>"
            "<td style='width:100%'>"
               "<div style='text-align:center;font-weight:bold;font-size:62px'>") +
                  + (myOptions->powerOn ? F("ON") : F("OFF")) +
               F("</div>"
            "</td>"
         "</tr>"
      "</table>");
   
   server.send(200, F("text/html"), info);
}

/** Load the firmware update page after starting OTA. */
void MyWebServer::loadUpdate()
{
   if (!myOptions || !myData) {
      return;
   }
   if (!myData->isOtaActive) {
      SetupOTA();
      ArduinoOTA.begin();
      myData->isOtaActive = true;
   }

   if (loadFromSpiffs(F("/Update.html"))) {
      return;
   }
   handleNotFound();
}

/** Load the Settings page. */
void MyWebServer::loadSettings()
{
   if (loadFromSpiffs(F("/Settings.html"))) {
      return;
   }
   handleNotFound();
}

/** Sends the ajax settings detail information to the client. */
void MyWebServer::handleLoadSettingsInfo()
{
   if (!myOptions || !myData) {
      return;
   }
   
   String info;

   MyDbg(F("LoadSettings"), true);

   AddOption(info, F("isDebugActive"), F("Debug Active"), myOptions->isDebugActive);

   AddOption(info, F("bme280CheckIntervalSec"), F("Temperature check every (Interval)"), formatInterval(myOptions->bme280CheckIntervalSec));

#ifdef SIM808_CONNECTED
   AddOption(info, F("gprsAP"),       F("GPRS AP"),       myOptions->gprsAP);
   AddOption(info, F("gprsUser"),     F("GPRS User"),     myOptions->gprsUser);
   AddOption(info, F("gprsPassword"), F("GPRS Password"), myOptions->gprsPassword);
#endif

   AddBr(info);
   {
      HtmlTag fieldset(info, F("fieldset"));
      {
         HtmlTag legend(info, F("legend"));

         AddOption(info, F("connectWifiAP"), F("WiFi connect"), myOptions->connectWifiAP, false);
      }

      AddOption(info, F("wifiAP"), F("WiFi SSID"), myOptions->wifiAP, false);
      AddOption(info, F("wifiPassword"), F("WiFi Password"), myOptions->wifiPassword, false, true);
   }

#ifdef SIM808_CONNECTED
   AddBr(info);
   {
      HtmlTag fieldset(info, F("fieldset"));
      {
         HtmlTag legend(info, F("legend"));

         AddOption(info, F("isSmsEnabled"), F("SMS Enabled"), myOptions->isSmsEnabled, false);
      }

      AddOption(info, F("smsCheckIntervalSec"), F("SMS check every (Interval)"), formatInterval(myOptions->smsCheckIntervalSec));
      AddOption(info, F("phoneNumber"),         F("Information send to"),        myOptions->phoneNumber, false);
   }
   AddBr(info);
   {
      HtmlTag fieldset(info, F("fieldset"));
      {
         HtmlTag legend(info, F("legend"));

         AddOption(info, F("isGpsEnabled"), F("GPS Enabled"), myOptions->isGpsEnabled, false);
      }
      AddOption(info, F("gpsCheckIntervalSec"), F("GPS check every (Interval)"),         formatInterval(myOptions->gpsCheckIntervalSec));
      AddOption(info, F("gpsTimeoutSec"),       F("GPS timeout"),                        formatInterval(myOptions->gpsTimeoutSec));
      AddOption(info, F("minMovingDistance"),   F("GPS is moving if more than (meter)"), String(myOptions->minMovingDistance), false);
   }
#endif

   AddBr(info);
   {
      HtmlTag fieldset(info, F("fieldset"));
      {
         HtmlTag legend(info, F("legend"));
         
         AddOption(info, F("isMqttEnabled"), F("MQTT Active"), myOptions->isMqttEnabled, false);
      }

      AddOption(info, F("mqttName"),                  F("MQTT Name"),                              myOptions->mqttName);
      AddOption(info, F("mqttId"),                    F("MQTT Id"),                                myOptions->mqttId);

      AddOption(info, F("mqttServer"),                F("MQTT Server"),                            myOptions->mqttServer);
      AddOption(info, F("mqttPort"),                  F("MQTT Port"),                              String(myOptions->mqttPort));
      AddOption(info, F("mqttUser"),                  F("MQTT User"),                              myOptions->mqttUser);
      AddOption(info, F("mqttPassword"),              F("MQTT Password"),                          myOptions->mqttPassword, true, true);
#ifdef SIM808_CONNECTED
      AddOption(info, F("mqttSendOnMoveEverySec"),    F("MQTT Send on moving every (Interval)"),   formatInterval(myOptions->mqttSendOnMoveEverySec));
      AddOption(info, F("mqttSendOnNonMoveEverySec"), F("MQTT Send on standing every (Interval)"), formatInterval(myOptions->mqttSendOnNonMoveEverySec), false);
#else
      AddOption(info, F("mqttSendOnNonMoveEverySec"), F("MQTT Send every (Interval)"),             formatInterval(myOptions->mqttSendOnNonMoveEverySec), false);
#endif
   }

   AddBr(info);
   {
      HtmlTag fieldset(info, F("fieldset"));
      {
         HtmlTag legend(info, F("legend"));

         AddOption(info, F("isDeepSleepEnabled"), F("Power saving mode active"), myOptions->isDeepSleepEnabled, false);
      }
      AddOption(info, F("powerSaveModeVoltage"),  F("Power saving mode below (Volt)"), String(myOptions->powerSaveModeVoltage, 2));
      AddOption(info, F("powerCheckIntervalSec"), F("Check power every (Interval)"),   formatInterval(myOptions->powerCheckIntervalSec));

      AddOption(info, F("activeTimeSec"),    F("Active time (Interval)"),    formatInterval(myOptions->activeTimeSec));
      AddOption(info, F("deepSleepTimeSec"), F("DeepSleep time (Interval)"), formatInterval(myOptions->deepSleepTimeSec), false);
   }

   AddIntervalInfo(info);

   server.send(200, F("text/html"), info);
}

/** Reads all the options from the url and save them to the SPIFFS. */
void MyWebServer::handleSaveSettings()
{
   if (!myOptions || !myData) {
      return;
   }
   
   MyDbg(F("SaveSettings"), true);
   GetOption(F("gprsAP"),                    myOptions->gprsAP);
   GetOption(F("gprsUser"),                  myOptions->gprsUser);
   GetOption(F("gprsPassword"),              myOptions->gprsPassword);
   GetOption(F("connectWifiAP"),             myOptions->connectWifiAP);
   GetOption(F("wifiAP"),                    myOptions->wifiAP);
   GetOption(F("wifiPassword"),              myOptions->wifiPassword);
   GetOption(F("isDebugActive"),             myOptions->isDebugActive);
   GetOption(F("bme280CheckIntervalSec"),    myOptions->bme280CheckIntervalSec);
   GetOption(F("isSmsEnabled"),              myOptions->isSmsEnabled);
   GetOption(F("phoneNumber"),               myOptions->phoneNumber);
   GetOption(F("smsCheckIntervalSec"),       myOptions->smsCheckIntervalSec);
   GetOption(F("isGpsEnabled"),              myOptions->isGpsEnabled);
   GetOption(F("gpsTimeoutSec"),             myOptions->gpsTimeoutSec);
   GetOption(F("gpsCheckIntervalSec"),       myOptions->gpsCheckIntervalSec);
   GetOption(F("minMovingDistance"),         myOptions->minMovingDistance);
   GetOption(F("isDeepSleepEnabled"),        myOptions->isDeepSleepEnabled);
   GetOption(F("powerSaveModeVoltage"),      myOptions->powerSaveModeVoltage);
   GetOption(F("powerCheckIntervalSec"),     myOptions->powerCheckIntervalSec);
   GetOption(F("activeTimeSec"),             myOptions->activeTimeSec);
   GetOption(F("deepSleepTimeSec"),          myOptions->deepSleepTimeSec);
   GetOption(F("isMqttEnabled"),             myOptions->isMqttEnabled);
   GetOption(F("mqttName"),                  myOptions->mqttName);
   GetOption(F("mqttId"),                    myOptions->mqttId);
   GetOption(F("mqttServer"),                myOptions->mqttServer);
   GetOption(F("mqttPort"),                  myOptions->mqttPort);
   GetOption(F("mqttUser"),                  myOptions->mqttUser);
   GetOption(F("mqttPassword"),              myOptions->mqttPassword);
   GetOption(F("mqttSendOnMoveEverySec"),    myOptions->mqttSendOnMoveEverySec);
   GetOption(F("mqttSendOnNonMoveEverySec"), myOptions->mqttSendOnNonMoveEverySec);

   // Reset the rtc data if something has changed.
   myData->awakeTimeOffsetSec = millis() / 1000;
   myOptions->save();

   if (false /* reboot */) {
      myData->restartInfo = 
         F("<b>Settings saved</b>");
      loadRestart();
   }
   loadMain();
}

/** Load the detail info part via ajax call. */
void MyWebServer::handleLoadInfoInfo()
{
   if (!myOptions || !myData) {
      return;
   }
   
   String info;
   String ssidRssi = (String) myOptions->wifiAP + F(" (") + WifiGetRssiAsQuality(WiFi.RSSI()) + F("%)");

   AddTableBegin(info);
   if (myData->status != "") {
      AddTableTr(info, F("Status"), myData->status);
      AddTableTr(info);
   }
   if (myData->isOtaActive) {
      AddTableTr(info, F("OTA"), F("Active"));
      AddTableTr(info);
   }
   if (ssidRssi != "" || myData->softAPIP || myData->softAPmacAddress != "" || myData->stationIP != "") {
      AddTableTr(info, F("AP1 SSID (RSSI)"),   ssidRssi);
      AddTableTr(info, F("AP IP"),             myData->softAPIP);
      AddTableTr(info, F("Locale IP"),         myData->stationIP);
      AddTableTr(info, F("MAC Address"),       myData->softAPmacAddress);
      AddTableTr(info);
   }
#ifdef SIM808_CONNECTED
   if (myData->modemInfo     != "" || myData->modemIP != ""      || myData->imei        != "" || myData->cop != "" || 
       myData->signalQuality != "" || myData->batteryLevel != "" || myData->batteryVolt != "") {
      AddTableTr(info, F("Modem Info"),        myData->modemInfo);
      AddTableTr(info, F("Modem IP"),          myData->modemIP);
      AddTableTr(info, F("IMEI"),              myData->imei);
      AddTableTr(info, F("COP"),               myData->cop);
      AddTableTr(info, F("Signal Quality"),    myData->signalQuality);
      AddTableTr(info, F("Battery Level"),     myData->batteryLevel);
      AddTableTr(info, F("Battery Volt"),      myData->batteryVolt);
      AddTableTr(info);
   }
   if (myData->rtcData.lastGps.fixStatus) {
      AddTableTr(info, F("Longitude"), myData->rtcData.lastGps.longitudeString());
      AddTableTr(info, F("Latitude"),  myData->rtcData.lastGps.latitudeString());
      AddTableTr(info, F("Altitude"),  myData->rtcData.lastGps.altitudeString());
      AddTableTr(info, F("Km/h"),      myData->rtcData.lastGps.kmphString());
      AddTableTr(info, F("Satellite"), myData->rtcData.lastGps.satellitesString());
      AddTableTr(info, F("Course"),    myData->rtcData.lastGps.courseString());
      AddTableTr(info, F("GPS Date"),  myData->rtcData.lastGps.date.dateString());
      AddTableTr(info, F("GPS Time"),  myData->rtcData.lastGps.time.timeString());
      AddTableTr(info);
   }
   if (myData->isMoving || myData->movingDistance != 0.0) {
      AddTableTr(info, F("Moving"), myData->isMoving ? F("Yes") : F("No"));
      AddTableTr(info, F("Distance (m)"), String(myData->movingDistance, 2));
      AddTableTr(info);
   }
   AddTableTr(info, F("Active Time"),          formatInterval(myData->getActiveTimeSec()));
   AddTableTr(info, F("PowerUpTime"),          formatInterval(myData->getPowerOnTimeSec()));
   AddTableTr(info, F("DeepSleepTime"),        formatInterval(myData->rtcData.deepSleepTimeSec));
   AddTableTr(info, F("mAh"),                  String(myData->getPowerConsumption(), 2));
   AddTableTr(info, F("Low power mAh"),        String(myData->getLowPowerPowerConsumption(), 2));
   AddTableTr(info);                       
#endif   
   AddTableTr(info, F("ESP Chip ID"),          String(ESP.getChipId()));
   AddTableTr(info, F("Flash Chip ID"),        String(ESP.getFlashChipId()));
   AddTableTr(info, F("Real Flash Memory"),    String(ESP.getFlashChipRealSize()) + F(" Byte"));
   AddTableTr(info, F("Total Flash Memory"),   String(ESP.getFlashChipSize())     + F(" Byte"));
   AddTableTr(info, F("Used Flash Memory"),    String(ESP.getSketchSize())        + F(" Byte"));
   AddTableTr(info, F("Free Sketch Memory"),   String(ESP.getFreeSketchSpace())   + F(" Byte"));
   AddTableTr(info, F("Free Heap Memory"),     String(ESP.getFreeHeap())          + F(" Byte"));

   AddTableEnd(info);
   
   server.send(200, F("text/html"), info);
}

/** Load the console page */
void MyWebServer::loadConsole()
{
   if (loadFromSpiffs(F("/Console.html"))) {
      if (server.hasArg(F("clear"))) {
         myData->logInfos.removeAll();
      }
      return;
   }
   handleNotFound();
}

/** Handle the Console ajax calls to get AT commands and show the result of the calls or debug informations. */
void MyWebServer::handleLoadConsoleInfo()
{
   if (!myOptions || !myData) {
      return;
   }
   
   String sendData;
   String cmd      = server.arg(F("c1"));
   String startIdx = server.arg(F("c2"));

   if (server.hasArg(F("c1"))) {
      MyDbg(cmd, true);
      myData->consoleCmds.addTail(cmd);
   }

   int indexFrom = atoi(startIdx.c_str()) - myData->logInfos.rolledOut();

   sendData = (String) 
      F("<r>"
         "<i>") + String(myData->logInfos.count() + myData->logInfos.rolledOut()) + F("</i>"
         "<j>1</j>"
         "<l>");
            for (int i = indexFrom; i < myData->logInfos.count(); i++) {
               if (i >= 0 && i < myData->logInfos.count()) {
                  sendData += TextToUrl(myData->logInfos.getAt(i));
                  sendData += '\n';
               }
            }
   sendData += (String)
         F("</l>"
      "</r>");
      
   server.send(200, F("text/xml"), sendData.c_str());
}

/** Load the restart page. */
void MyWebServer::loadRestart()
{
   if (loadFromSpiffs(F("/Restart.html"))) {
      MyDbg(F("Load File /Restart.html"), true);
      MyDelay(2000);
      MyDbg(F("Restart"), true);
      ESP.restart();
      return;
   }
   handleNotFound();
}

/** Handle the restart ajax call of the restart reason. */
void MyWebServer::handleLoadRestartInfo()
{
   if (!myOptions || !myData) {
      return;
   }
   
   server.send(200, F("text/html"), myData->restartInfo);
   myData->restartInfo = "";
}

/** Handle if the url could not be found. */
void MyWebServer::handleNotFound()
{
   if (!myOptions || !myData) {
      return;
   }
   
   String message = F("File Not Found\n");
   
   message += F("URI: ");
   message += server.uri();
   message += F("\nMethod: ");
   message += (server.method() == HTTP_GET) ? F("GET") : F("POST");
   message += F("\nArguments: ");
   message += server.args();
   message += F("\n");
   // Better no debuggin of browser args
   // for (uint8_t i=0; i<server.args(); i++){
   //   message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
   // }
   server.send(404, F("text/plain"), message);
   MyDbg(message, true);
}

/** Default for an unknown web request on not found. */
void MyWebServer::handleWebRequests()
{
   if (loadFromSpiffs(server.uri())) {
      return;
   }
   handleNotFound();
}
