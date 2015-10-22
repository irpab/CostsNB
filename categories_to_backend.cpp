
#include <sstream>

#include "restclient-cpp/restclient.h"
#include "json/json-forwards.h"
#include "json/json.h"
#include "miniz_wrp.h"
#include "base64.h"
#include "utils.h"
#include "plog/Log.h"

#include "categories_to_backend.h"

//#include <QDebug>

enum HttpCodes {
  OK = 200
, CREATED = 201
, BAD_REQUEST = 400
, UNAUTHORIZED = 401
, NOT_FOUND = 404
};

CategoriesToRestfulBackend::CategoriesToRestfulBackend(CategoriesToJsonConverter *categories_to_json_converter_
    , AbstractConfig* cfg_)
  : categories_to_json_converter(categories_to_json_converter_)
  , cfg(cfg_)
{
  server_addr = cfg->GetValue("backend", "server_addr");
  user_name = cfg->GetValue("backend", "user_name");
  password = cfg->GetValue("backend", "password");
  token = cfg->GetValue("backend", "token");

  auto log_dir = cfg->GetValue("log", "log_dir");
  auto log_file_name = log_dir + "/costs_nb.log";
  const unsigned int log_file_size = 50000;
  const unsigned short log_file_count = 2;
  plog::init(plog::debug, log_file_name.c_str(), log_file_size, log_file_count);

  token_url   = server_addr + "/costs_nb/token";
  save_db_url = server_addr + "/costs_nb/save_db";
}

std::string CategoriesToRestfulBackend::PrepareDataForSending(CategoriesElem *categories)
{
  auto json_str = categories_to_json_converter->CategoriesToJsonStr(categories);
  auto zipped_str = compress_string(json_str);
  auto base64_str = base64_encode(reinterpret_cast<const unsigned char*>(zipped_str.c_str()), zipped_str.size());
  
  return base64_str;
}

bool CategoriesToRestfulBackend::SyncToBackend(CategoriesElem *categories)
{
  LOG_DEBUG << "start syncing";
  auto data = PrepareDataForSending(categories);
  std::string json_request = "{\
    \"zipped\": true,\
    \"data\": \"" + data + "\"\
  }";

  SetTokenAuth();
  RestClient::response r = RestClient::put(save_db_url, "application/json", json_request);
  if (r.code == HttpCodes::CREATED) {
    LOG_DEBUG << "synced with token\n";
    return true;
  }

  if (r.code != HttpCodes::UNAUTHORIZED)
    return false;

  cfg->SetValue("backend", "token", "");

  RequestToken();
  r = RestClient::put(save_db_url, "application/json", json_request);

  if (r.code != HttpCodes::CREATED)
    return false;

  LOG_DEBUG << "synced with token 2\n";
  return true;
}

bool CategoriesToRestfulBackend::RequestToken()
{
  SetPasswordAuth();
  RestClient::response r = RestClient::get(token_url);
  if (r.code != HttpCodes::OK)
    return false;

  Json::Value response_json;
  std::istringstream response_body(r.body);
  response_body >> response_json;

  token = response_json["token"].asString();
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
