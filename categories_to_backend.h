#ifndef CATEGORIES_TO_BACKEND_H
#define CATEGORIES_TO_BACKEND_H

#include "costs_nb_core.h"
#include "categories_to_json_converter.h"

class CategoriesToRestfulBackend : public CategoriesToBackend {
public:
  CategoriesToRestfulBackend(CategoriesToJsonConverter *categories_to_json_converter
    , AbstractConfig* cfg);
  bool SyncToBackend(CategoriesElem *categories);
private:
  std::string PrepareDataForSending(CategoriesElem * categories);

  CategoriesToJsonConverter *categories_to_json_converter;
  std::string server_addr;
  std::string user_name;
  std::string password;
  std::string token;
  AbstractConfig* cfg;

  std::string token_url;
  std::string save_db_url;

  bool RequestToken();
  void SetTokenAuth();
  void SetPasswordAuth();
};

#endif
