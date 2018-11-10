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
  * @file Debug.h
  * 
  * Helper functions for a MyDbg macro which can show all the debug information on the console page and
  * a helper function for a delay function who can update some webbrowser information in the background.
  */


 /** This function has to be overwritten to implement the handle of debug informations. */
void myDebugInfo(String info, bool isWebServer, bool newline);

/** Short version of myDebugInfo.
  * fromWebServer prevents recursive calls when from WebServer
  */
void MyDbg(String info, bool fromWebServer = false, bool newline = true) 
{
   myDebugInfo(info, fromWebServer, newline);
}


/** This function has to be overwritten to implement background delay calls. */
void myDelayLoop();

/** Replacement with background calls when we have to wait. Replace the delay function. */
void myDelay(long millisDelay)
{
   long m = millis();

   while (millis() - m < millisDelay) {
      myDelayLoop();
      yield();
      delay(1);
   }
}
