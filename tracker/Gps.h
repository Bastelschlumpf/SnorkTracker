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
  * @file Gps.h
  * 
  * GPS helper classes to parse the SIM808 gps information and calculate i.e. the distance between two positions.
  */

/**
  * Class to store the gps values with the right precision.
  */
class MyDegrees
{
public:
   uint16_t predecimal; //!< Value on the left side of the separator. 
   uint32_t billionths; //!< Value of the right side after the separator.
   bool     negative;   //!< Is the value negative?

public:
   MyDegrees();
   MyDegrees(const MyDegrees &myDegrees);

   double value();
   bool   set(const String &data);
};

/**
  * Class to store the two gps positions 
  */
class MyLocation
{
friend class MyGps;
protected:
   MyDegrees latitude_;  //!< Latitude
   MyDegrees longitude_; //!< Longitude

public:
   static double distanceBetween(double lat1, double long1, double lat2, double long2);
   static double courseTo(double lat1, double long1, double lat2, double long2);

public:
   double latitude();
   double longitude();

   String longitudeString();
   String latitudeString();

   double distanceTo(MyLocation &location);
   double courseTo  (MyLocation &location);
};

/**
  * Class to store the date with an integer.
  */
class MyDate
{
friend class MyGps;
protected:
   int date; //!< Date in the form of YearMonthDay i.e. 20170115
      
public:
   int year();
   int month();
   int day();

   MyDate() 
      : date(0)
   {
   }
};

/**
  * Class to store the time with an integer 
  */
class MyTime
{
friend class MyGps;
protected:
  int time; //!< Time in the form of HoursMinutesSecons i.e. 120135
  
public:
   int hour();
   int minute();
   int second();

   MyTime() 
      : time(0)
   {
   }
};
  
/**
  * GPS data class with all the data items from the GPS message from the SIM808 module
  */
class MyGps
{
public:
   bool       runStatus;         //!< Is the gps modul running?
   bool       fixStatus;         //!< Are the gps is valid received?
   MyDate     date;              //!< The received gps utc date.
   MyTime     time;              //!< The received gps utc time.
   MyLocation location;          //!< The gps position.
   double     altitude;          //!< The current height. 
   double     speed;             //!< The detected moving speed.
   double     course;            //!< The calculated course
   int        fixMode;           //!< Precission of the gps data.
   double     pdop;              //!< Dilution of precision
   double     hdop;              //!< Horizontal dilution of precision
   double     vdop;              //!< Vertical dilution of precision 
   int        satellitesInView;  //!< Sattelites in the View
   int        satellitesUsed;    //!< Sattelites used for gps position.

protected:
   bool parse(bool   &b, const String &data);
   bool parse(int    &i, const String &data);
   bool parse(double &d, const String &data);
   
public:
   MyGps();

   bool setRunStatus       (const String &data);
   bool setFixStatus       (const String &data);
   bool setDateTime        (const String &data);
   bool setLatitude        (const String &data);
   bool setLongitude       (const String &data);
   bool setAltitude        (const String &data);
   bool setSpeed           (const String &data);
   bool setCourse          (const String &data);
   bool setFixMode         (const String &data);
   bool setHdop            (const String &data);
   bool setPdop            (const String &data);
   bool setVdop            (const String &data);
   bool setSatellitesInView(const String &data);
   bool setSatellitesUsed  (const String &data);

   String longitudeString  ();
   String latitudeString   ();
   String altitudeString   ();
   String kmphString       ();
   String satellitesString ();
   String courseString     ();
   String dateString       ();
   String timeString       ();
};

/* ******************************************** */

/** Constructor */
MyDegrees::MyDegrees()
   : predecimal(0)
   , billionths(0)
   , negative(false)
{
}

/** Constructor */
MyDegrees::MyDegrees(const MyDegrees &myDegrees)
   : predecimal(myDegrees.predecimal)
   , billionths(myDegrees.billionths)
   , negative(myDegrees.negative)
{
}

/** Recalculate the value from the pre and post decimal to double. */
double MyDegrees::value()
{
   double ret = predecimal + billionths / 1000000000.0;

   return negative ? -ret : ret;
}

/** Sets the internal format from the nmea format. */
bool MyDegrees::set(const String &data)
{
   const char *term          = data.c_str();
   uint32_t    multiplier    = 1000000000UL;
   uint32_t    leftOfDecimal = (uint32_t) atol(term);

   predecimal = (int16_t)leftOfDecimal;
   billionths = 0;

   while (isdigit(*term)) {
      ++term;
   }

   if (*term == '.') {
      while (isdigit(*++term)) {
         multiplier /= 10;
         billionths += (*term - '0') * multiplier;
      }
   }
   negative = false;
   return true;
}

/** Calculate the distance between two gps positions in meter */
double MyLocation::distanceBetween(double lat1, double long1, double lat2, double long2)
{
   // returns distance in meters between two positions, both specified
   // as signed decimal-degrees latitude and longitude. Uses great-circle
   // distance computation for hypothetical sphere of radius 6372795 meters.
   // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
   // Courtesy of Maarten Lamers
   double delta = radians(long1 - long2);
   double sdlong = sin(delta);
   double cdlong = cos(delta);
   lat1 = radians(lat1);
   lat2 = radians(lat2);
   double slat1 = sin(lat1);
   double clat1 = cos(lat1);
   double slat2 = sin(lat2);
   double clat2 = cos(lat2);
   delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
   delta = sq(delta);
   delta += sq(clat2 * sdlong);
   delta = sqrt(delta);
   double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
   delta = atan2(delta, denom);
   return delta * 6372795;
}

/** Calculate the course between two gps positions */
double MyLocation::courseTo(double lat1, double long1, double lat2, double long2)
{
   // returns course in degrees (North=0, West=270) from position 1 to position 2,
   // both specified as signed decimal-degrees latitude and longitude.
   // Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
   // Courtesy of Maarten Lamers
   double dlon = radians(long2 - long1);
   lat1 = radians(lat1);
   lat2 = radians(lat2);
   double a1 = sin(dlon) * cos(lat2);
   double a2 = sin(lat1) * cos(lat2) * cos(dlon);
   a2 = cos(lat1) * sin(lat2) - a2;
   a2 = atan2(a1, a2);
   if (a2 < 0.0)
   {
      a2 += TWO_PI;
   }
   return degrees(a2);
}

/** Gets the latitude */
double MyLocation::latitude()
{
   return latitude_.value();
}

/** Gets the Longitude */
double MyLocation::longitude()
{
   return longitude_.value();
}

/** Returns the longitude as a string */
String MyLocation::longitudeString()
{
   return String(longitude(), 6);
}

/** Returns the latitude as a string */
String MyLocation::latitudeString()
{
   return String(latitude(), 6);
}

/** Calculate the distance between to another gps location */
double MyLocation::distanceTo(MyLocation &to)
{
   return distanceBetween(latitude(), longitude(), to.latitude(), to.longitude());
}

/** Calculate the course to another gps location */
double MyLocation::courseTo(MyLocation &to)
{
   return courseTo(latitude(), longitude(), to.latitude(), to.longitude());
}

/** Gets the year from the date */
int MyDate::year()
{
   return date / 10000;
}

/** Gets the Month from the date */
int MyDate::month()
{
   return (date / 100) % 100;
}

/** Gets the day from the date */
int MyDate::day()
{
   return date % 100;
}

/** Gets the hour from the time value */
int MyTime::hour()
{
   return time / 10000;
}

/** Gets the minute from the time value */
int MyTime::minute()
{
   return (time / 100) % 100;
}

/** Gets the seconds from the time value */
int MyTime::second()
{
   return time % 100;
}


/** Constructor */
MyGps::MyGps()
   : runStatus(false)
   , fixStatus(false)
   , altitude(0)
   , speed(0)
   , course(0)
   , fixMode(0)
   , hdop(0)
   , pdop(0)
   , vdop(0)
   , satellitesInView(0)
   , satellitesUsed(0)
{
}

/** Parse a bool value from a string '0' or '1' */
bool MyGps::parse(bool &b, const String &data)
{
   if (data == "0") {
      b = false;
      return true;
   } else if (data == "1") {
      b = true;
      return true;
   }
   return false;
}

/** Parse a int value from a string */
bool MyGps::parse(int &i, const String &data)
{
    i = atol(data.c_str());
    return true;
}

/** Parse a double value from a string */
bool MyGps::parse(double &d, const String &data)
{
    d = atof(data.c_str());
    return true;
}

/** Sets the run status from the data string */
bool MyGps::setRunStatus(const String &data)
{
   return parse(runStatus, data);
}

/** Sets the fix status from the data string */
bool MyGps::setFixStatus(const String &data)
{
   return parse(fixStatus, data);
}

/** Sets the date and time from the data string */
bool MyGps::setDateTime(const String &data)
{
   parse(date.date, data.substring(0, 8));
   parse(time.time, data.substring(8));
   return true;
}

/** Set the latitude from the data string */
bool MyGps::setLatitude(const String &data)
{
   return location.latitude_.set(data);
}

/** Sets the longitude from the data string */
bool MyGps::setLongitude(const String &data)
{
   return location.longitude_.set(data);
}

/** Sets the altitude from the data string */
bool MyGps::setAltitude(const String &data)
{
   return parse(altitude, data);
}

/** Sets the speed value from the data string */
bool MyGps::setSpeed(const String &data)
{
   return parse(speed, data);
}

/** Sets the course value from the data string */
bool MyGps::setCourse(const String &data)
{
   return parse(course, data);
}

/** Sets the fix mode from the data string */
bool MyGps::setFixMode(const String &data)
{
   return parse(fixMode, data);
}

/** Sets the pdop mode from the data string */
bool MyGps::setPdop(const String &data)
{
   return parse(pdop, data);
}

/** Sets the hdop mode from the data string */
bool MyGps::setHdop(const String &data)
{
   return parse(hdop, data);
}

/** Sets the vdop mode from the data string */
bool MyGps::setVdop(const String &data)
{
   return parse(vdop, data);
}

/** Sets the satellites in view value from the data string */
bool MyGps::setSatellitesInView(const String &data)
{
   return parse(satellitesInView, data);
}

/** Sets the satellites used value from the data string */
bool MyGps::setSatellitesUsed(const String &data)
{
   return parse(satellitesUsed, data);
}

/** Returns the longitude as a string */
String MyGps::longitudeString()
{
   return location.longitudeString();
}

/** Returns the latitude as a string */
String MyGps::latitudeString()
{
   return location.latitudeString();
}

/** Returns the altitude as a string */
String MyGps::altitudeString()
{
   return String(altitude, 0);
}

/** Returns the kmph as a string */
String MyGps::kmphString()
{
   return String(speed, 0);
}

/** Returns the satellites as a string */
String MyGps::satellitesString()
{
   return String(satellitesUsed);
}

/** Returns the course as a string */
String MyGps::courseString()
{
   return String(course);
}

/** Returns the gps date as a string */
String MyGps::dateString()
{
   return String(date.day()) + '-' + String(date.month()) + '-' + String(date.year());
}

/** Returns the gps time as a string */
String MyGps::timeString()
{
   String(time.hour()) + ':' + String(time.minute()) + ':' + String(time.second());
}
