
#include "utils.h"

namespace utils
{

std::string MonthNumToStr(const unsigned short month)
{
    switch (month) {
    case  1: return "Jan";
    case  2: return "Feb";
    case  3: return "Mar";
    case  4: return "Apr";
    case  5: return "May";
    case  6: return "Jun";
    case  7: return "Jul";
    case  8: return "Aug";
    case  9: return "Sep";
    case 10: return "Oct";
    case 11: return "Nov";
    case 12: return "Dec";
    default: return "---";
    }
}

unsigned short MonthStrToNum(const std::string &month)
{
       if (0 == std::string("Jan").compare(month)) return  1;
  else if (0 == std::string("Feb").compare(month)) return  2;
  else if (0 == std::string("Mar").compare(month)) return  3;
  else if (0 == std::string("Apr").compare(month)) return  4;
  else if (0 == std::string("May").compare(month)) return  5;
  else if (0 == std::string("Jun").compare(month)) return  6;
  else if (0 == std::string("Jul").compare(month)) return  7;
  else if (0 == std::string("Aug").compare(month)) return  8;
  else if (0 == std::string("Sep").compare(month)) return  9;
  else if (0 == std::string("Oct").compare(month)) return 10;
  else if (0 == std::string("Nov").compare(month)) return 11;
  else if (0 == std::string("Dec").compare(month)) return 12;
  return 0;
}

struct tm * Now()
{
    time_t t = time(0);
    return localtime(&t);
}

bool FileExists(const std::string& fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

std::string EscapeJsonString(const std::string& input) {
  std::ostringstream ss;
  for (auto iter = input.cbegin(); iter != input.cend(); iter++) {
    switch (*iter) {
      case '\\': ss << "\\\\"; break;
      case '"': ss << "\\\""; break;
      case '/': ss << "\\/"; break;
      case '\b': ss << "\\b"; break;
      case '\f': ss << "\\f"; break;
      case '\n': ss << "\\n"; break;
      case '\r': ss << "\\r"; break;
      case '\t': ss << "\\t"; break;
      case '\u0019': ss << ""; break;
      case '\u0018': ss << ""; break;
      case '\u0000': ss << ""; break;
      default: ss << *iter; break;
    }
  }
  return ss.str();
}

} // namespace utils
