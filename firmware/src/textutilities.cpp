//#include <cmath>
#include "textutilities.h"

//float tofloat(const char * s)
//{
//  float sp = ((s[1] - '0')*10 +
//              (s[2] - '0')) +
//             ((s[4] - '0')*0.1f +
//              (s[5] - '0')*0.01f);
//  return (s[0] == '-') ? -sp : sp;
//}

float tofloat(const char * s)
{
  // Define and initialize variables for checking the sign of the number
  int index = 0;           // index of a character in the array
  char c = s[0];           // a character from the character array
  bool is_positive = true; // true if the number is positive, false otherwise
  
  // Check the sign of the number
  if (c == '+')          // character array starts with '+'
  {
    index = 1;           // don't read value of first character (since it is a '+')
  }
  else if (c == '-')     // character array starts with '-'
  {
    index = 1;           // don't read value of first character (since it is a '-')
    is_positive = false; // the number is negative
  }
  
  // Define and initialize variables for converting the character array to a positive number
  float number = 0.0f;         // the number in the character array (without sign)
  bool is_decimal = false;     // becomes true once a dot has been detected, meaning the number is decimal
  float decimal_weight = 0.1f; // weight of a character after the dot (e.g. the 6 in 1.56 has weight 0.01)
  
  // Loop over the character array to convert it to a positive number 
  c = s[index];
  while (c != '\0')           // character array ends with null terminator '\0'
  {
    // Parse the current character
    if ((c>='0') && (c<='9')) // the current character is a number
    {
      // Update the value of the number
      if (!is_decimal)        // no dot has been found yet
      {
        number *= 10;         // multiply the number by 10 | e.g. 156 = 10*15 ...
        number += c - '0';    // add the character value   |                  ... + 6
      }
      else                    // a dot has been found
      {
        number += decimal_weight*(c - '0'); // e.g. 156.78 = 156.7 + 0.01*8s
        decimal_weight /= 10; // decrease weight, since the next character will contribute 10 times less
      }
    }
    else if (c == '.')        // current character is a dot
    {
      is_decimal = true;
    }
    
    // Update the index and retrieve the next character
    index++;
    c = s[index];
  }
  
  // Return the number with the correct sign
  return (is_positive ? number : -number);
}

