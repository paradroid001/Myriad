#ifndef MYRIADEXAMPLE_BASICLOGGER_H
#define MYRIADEXAMPLE_BASICLOGGER_H

#include "myriad.h"
#include <iostream>
#include <sstream>
#include <cstring> //for c strings
#include <cstdio>  //for sscanf
#include <unordered_map>
#include <string>

struct InsertionPoint
{
  int strpos_start;
  int strpos_end;
  int index;
};

class BasicLogger : public Myriad::ILogger
{
public:
  virtual void Log(Myriad::MyrLogLevel_t level, const char *fmt, ...) override
  {
    std::unordered_map<int, InsertionPoint> args_map;
    va_list args;
    va_start(args, fmt); // args starts after fmt.
    // count the number of times {n} appears in the fmt string
    // and build the va_strings map.
    int pos = 0;
    while (pos > -1)
    {
      InsertionPoint p = GetNextInsertionPoint(fmt + pos);
      if (p.index != -1)
      {
        args_map[p.index] = p;
        std::cout << "Got index " << p.index << std::endl;
        pos += p.strpos_end; // cumulative!
      }
      else
        pos = p.index; // this will end the loop
    }

    // print the insertion points.
    for (auto &record : args_map)
    {
      std::cout << record.first << "[ " << "start: " << record.second.strpos_start << " end: " << record.second.strpos_end << " index: " << record.second.index << std::endl;
    }

    // build the final string.
    std::ostringstream out;
    int str_start = 0;
    for (unsigned int i = 0; i < args_map.size(); i++)
    {
      InsertionPoint ipoint = args_map[i];
      std::string s;
      s.append(fmt + str_start, ipoint.strpos_start);
      out << s;
      out << "??"; // va_arg();
      str_start = ipoint.strpos_end;
      s.clear();
    }
    va_end(args);
    std::cout << out.str() << std::endl;
  }

  InsertionPoint GetNextInsertionPoint(const char *fmt)
  {
    InsertionPoint ret = {-1, -1, -1};
    int len = strlen(fmt);
    for (int index = 0; (index < len) && ret.index == -1; ++index)
    {
      if (*(fmt + index) == '{' && ((index + 2) < len) && *(fmt + index + 2) == '}')
      {
        // is the thing inside {} a number?
        int num;
        char tmpstr[2];
        tmpstr[0] = *(fmt + index + 1);
        tmpstr[1] = '\0';

        if (1 == sscanf(tmpstr, "%d", &num))
        {
          // it was a number. Set up the struct.
          ret.index = num;
          ret.strpos_start = index;
          ret.strpos_end = index + 2;
          std::cout << "tmpstr: " << tmpstr << std::endl;
          std::cout << "num was " << num << std::endl;
        }
      }
    }
    return ret;
  }
};

#endif
