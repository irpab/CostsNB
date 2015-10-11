
#include <sstream>

#include "restclient-cpp/restclient.h"
#include "json/json-forwards.h"
#include "json/json.h"
#include "miniz_wrp.h"
#include "base64.h"
#include "utils.h"

#include "categories_to_backend.h"

//#include <QDebug>

enum HttpCodes {
  OK = 200
, CREATED = 201
, BAD_REQUEST = 400
, UNAUTHORIZED = 401
, NOT_FOUND = 404
};

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
    , AbstractConfig* cfg_)
  : categories_to_json_converter(categories_to_json_converter_)
  , cfg(cfg_)
{
  server_addr = cfg->GetValue("backend", "server_addr");
  user_name = cfg->GetValue("backend", "user_name");
  password = cfg->GetValue("backend", "password");
  token = cfg->GetValue("backend", "token");

//  qDebug() << "server_addr = " << QString::fromStdString(server_addr);
//  qDebug() << "user_name = " << QString::fromStdString(user_name);
//  qDebug() << "password = " << QString::fromStdString(password);
//  qDebug() << "token = " << QString::fromStdString(token);

  token_url   = server_addr + "/costs_nb/token";
  save_db_url = server_addr + "/costs_nb/save_db";
}

std::string CategoriesToRestfulBackend::PrepareDataForSending(CategoriesElem *categories)
{
  auto json_str = categories_to_json_converter->CategoriesToJsonStr(categories);
  // auto escaped_str = EscapeJsonString(json_str);
  auto escaped_str = json_str;
  auto zipped_str = compress_string(escaped_str);
  auto base64_str = base64_encode(reinterpret_cast<const unsigned char*>(zipped_str.c_str()), zipped_str.size());
  
  return base64_str;
}

bool CategoriesToRestfulBackend::SyncToBackend(CategoriesElem *categories)
{
  auto data = PrepareDataForSending(categories);
  std::string json_request = "{\
    \"zipped\": true,\
    \"data\": \"" + data + "\"\
  }";

//  qDebug() << "SyncToBackend: try first";
  SetTokenAuth();
  RestClient::response r = RestClient::put(save_db_url, "application/json", json_request);
//  qDebug() << "SyncToBackend: r.code " << r.code;
  if (r.code == HttpCodes::CREATED)
    return true;

  if (r.code != HttpCodes::UNAUTHORIZED)
    return false;

  cfg->SetValue("backend", "token", "");

//  qDebug() << "SyncToBackend: RequestToken";
  RequestToken();
  r = RestClient::put(save_db_url, "application/json", json_request);
//  qDebug() << "SyncToBackend: r.code = " << r.code;

  if (r.code != HttpCodes::CREATED)
    return false;

  return true;
}

bool CategoriesToRestfulBackend::RequestToken()
{
  SetPasswordAuth();
  RestClient::response r = RestClient::get(token_url);
//  qDebug() << "SyncToBackend: r.code " << r.code;
  if (r.code != HttpCodes::OK)
    return false;

  Json::Value response_json;
  std::istringstream response_body(r.body);
  response_body >> response_json;

  token = response_json["token"].asString();
//  qDebug() << "SyncToBackend: token " << QString::fromStdString(token);
  cfg->SetValue("backend", "token", token);
  SetTokenAuth();
  return true;
}

void CategoriesToRestfulBackend::SetTokenAuth()
{
  RestClient::setAuth(token, "no_password");
}

void CategoriesToRestfulBackend::SetPasswordAuth()
{
  RestClient::setAuth(user_name, password);
}
