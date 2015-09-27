#ifndef CATEGORIES_TO_BACKEND_H
#define CATEGORIES_TO_BACKEND_H

#include "costs_nb_core.h"
#include "categories_to_json_converter.h"

class CategoriesToRestfulBackend : public CategoriesToBackend {
public:
  CategoriesToRestfulBackend(CategoriesToJsonConverter *categories_to_json_converter
    , const std::string &server_addr
    , const std::string &user_name
    , const std::string &password);
  bool SyncToBackend(CategoriesElem *categories);
private:
  CategoriesToJsonConverter *categories_to_json_converter;
  std::string server_addr;
  std::string user_name;
  std::string password;
};

#endif
