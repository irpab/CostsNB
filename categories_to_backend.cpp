
#include <sstream>

#include "restclient-cpp/restclient.h"

#include "categories_to_backend.h"

#include <QDebug>

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

CategoriesToRestfulBackend::CategoriesToRestfulBackend(CategoriesToJsonConverter *categories_to_json_converter_
    , const std::string &server_addr_
    , const std::string &user_name_
    , const std::string &password_)
  : categories_to_json_converter(categories_to_json_converter_)
  , server_addr(server_addr_)
  , user_name(user_name_)
  , password(password_)
{
  RestClient::setAuth(user_name, password);
}

bool CategoriesToRestfulBackend::SyncToBackend(CategoriesElem *categories)
{
  auto json_str = categories_to_json_converter->CategoriesToJsonStr(categories);

  std::string save_db_url = server_addr + "/costs_nb/save_db";
  std::string json_request = "{\"data\": \"" + EscapeJsonString(json_str) + "\"}";

  RestClient::response r = RestClient::put(save_db_url, "application/json", json_request);
  qDebug() << "r.code = " << r.code;

  if (r.code != 201) {
    return false;
  }

  return true;
}
