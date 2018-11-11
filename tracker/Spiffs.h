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
  * @file Spiffs.h
  * 
  * Helper-function for the SPIFF.
  */

/** Read the content of a file from the SPIFF file-system. */
String readFromSpiffs(String path)
{
   String ret;
   File   file = SPIFFS.open(path.c_str(), "r");
   
   if (!file) {
      MyDbg("SPIFFS File not found: '" + path + "'");
   } else {
      ret = file.readString();
      file.close();
   }
   return ret;
}
