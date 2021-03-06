/*! \file Ardusamber.h
 *
 * This is the main header file for the %Ardusamber library.
 *
 * This is library is for using Devine Lu Linvega's time format 'Desamber' with Arduino.
 * More information about Desamber here: https://wiki.xxiivv.com/#clock
 * */

#include "Arduino.h"

class Ardusamber
{
  public:
    void update();
    String getTime();
    String getFormattedTime();
    String getFormattedDate();
  private:
    String _beat;
    String _pulse;
    String addMissingDigits(String value, int digitsRequired);
    int calculateDayOfYear(int day, int month, int year);

    unsigned long _millisOffset;
    unsigned long _secondPrev;
    unsigned long _secondCur;
    unsigned long _msSinceMidnight;
    double _dtCalc;
    char _dtFormatting[9];

    String _prevFormattedDate;
    String _prevFormattedDateRef;
    String _thisDateRef;
};
