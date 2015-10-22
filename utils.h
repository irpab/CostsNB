#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <sstream>
#include <string>
#include <ctime>

namespace utils
{
  template < typename T > std::string to_string( const T& n )
  {
    std::ostringstream stm ;
    stm << n ;
    return stm.str() ;
  }

  std::string  MonthNumToStr(const unsigned short month);
  unsigned short MonthStrToNum(const std::string &month);
  struct tm * Now();
  bool FileExists(const std::string& fileName);
  std::string EscapeJsonString(const std::string& input);

}

#endif // UTILS_H

