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
  * @file StringList.h
  *
  * Class to store and load strings in a list.
  * It works internally with a big string and separators.
  */


#define MAX_LOG_INFOS_SIZE 1000 //!< Maximum bytes of the complete list items + separators.

/**
  * String List class. 
  * Internally all items are stored in one string with '\1' as separator.
  * The list has a maximum internal storage. While appending items it deletes
  * automatically from the beginning until it fits.
  * The performance could be optimized.
  */
class StringList
{
public:
   String infos;          //!< All the items in one string.
   int    infosCount;     //!< Number of items in the list.
   int    infosRolledOut; //!< Number of items rolled out.
   
public:
   StringList();

   bool   isEmpty();
   int    count(); 
   int    rolledOut();
   
   void   removeAll();
   
   String getAt(int idx);
   void   addTail(String newInfo);
   
   String removeHead();
   String removeTail();
}; 

/* ******************************************** */

StringList::StringList()
   : infosCount(0)
   , infosRolledOut(0)
{
}

/** Is the list empty? */
bool StringList::isEmpty()
{
   return infosCount == 0;
}

/** How many items are in the list? */
int StringList::count()
{ 
   return infosCount;
}

/** How many items are in the list? */
int StringList::rolledOut()
{
   return infosRolledOut;
}

/** Removes all items from the list. */
void StringList::removeAll()
{
   infos          = "";
   infosCount     = 0;
   infosRolledOut = 0;
}

/** Returns the n'th item from the list. */
String StringList::getAt(int idx)
{
   int currIdx = 0;
   int lastPos = 0;
   
   for(int i = 0; i < infos.length(); i++) {
      if (infos[i] == '\1') {
         if (currIdx == idx) {
            return infos.substring(lastPos, i);
         }
         lastPos = i + 1;
         currIdx++;
      }
   }
   return "";   
}

/** Append one item at the end of the list. 
  * If the list-string is too big then first items are deleted until it fits. 
  */
void StringList::addTail(String newInfo)
{
   while (infos.length() + newInfo.length() + 1 > MAX_LOG_INFOS_SIZE) {
      removeHead();
   }
   infos += newInfo;
   infos += '\1';
   infosCount++;
}

/** Remove the first item from the list. */
String StringList::removeHead()
{
   String ret;
   int    idx = infos.indexOf('\1');

   if (idx != -1) {
      ret   = infos.substring(0, idx);
      infos = infos.substring(idx + 1);
      infosCount--;
      infosRolledOut++;
   }
   return ret;
}

/** Removes the last item from the list. */
String StringList::removeTail()
{
   String ret;

   for (int i = infos.length() - 1; i >= 0; i--) {
      if (infos[i] == '\1') {
         ret   = infos.substring(i + 1);
         infos = infos.substring(0, i);
         infosCount--;
         break;
      }
   }
   return ret;
}
