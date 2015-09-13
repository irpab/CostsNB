
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

// std::string tm2str(const struct tm * t)
// {
//     std::stringstream ss;
//     ss << (t->tm_year + 1900) << '-' << utils::MonthNumToStr(t->tm_mon + 1) << '-'
//         << std::setw(2) << std::setfill('0') << t->tm_mday
//         << " " << t->tm_hour << ":" << std::setw(2) << std::setfill('0') << t->tm_min;
//     return ss.str();
// }

// std::string now2str()
// {
//     struct tm * t = utils::Now();
//     return tm2str(t);
// }

} // namespace utils
