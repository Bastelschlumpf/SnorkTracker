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
  * @file Config.h
  *
  * Public configuration information. 
  * Could be set before starting the programm or via web interface.
  */


#define WLAN_SID      "sid"        //!< WLAN SID
#define WLAN_PW       "password"   //!< WLAN password
#define GPRS_AP       "internet"   //!< gprs access point name
#define PHONE_NUMBER  "0123456789" //!< my phone number for sms communication

#define MQTT_NAME     "SIM808"     //!< MQTT name
#define MQTT_SERVER   "server"     //!< MQTT Server url
#define MQTT_PORT     1883         //!< MQTT Port (Default is 1883)
#define MQTT_USER     "user"       //!< MQTT connection user
#define MQTT_PASSWORD "password"   //!< MQTT connection password
#define MQTT_ID       "01"         //!< MQTT Modul id
