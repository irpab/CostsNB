#ifndef CATEGORIES_TO_JSON_CONVERTER_H
#define CATEGORIES_TO_JSON_CONVERTER_H

#include "json/json-forwards.h"
#include "json/json.h"

#include "costs_nb_core.h"


class CategoriesToJsonConverter {
public:
  // TODO: move semantics
  virtual std::string CategoriesToJsonStr(CategoriesElem *categories) = 0;
  virtual CategoriesElem * JsonStrToCategories(const std::string &json_str_categories) = 0;
  virtual ~CategoriesToJsonConverter() {}
};

class CategoriesToJsonConverterJsoncppLib : public CategoriesToJsonConverter {
public:
  // TODO: move semantics
  std::string CategoriesToJsonStr(CategoriesElem *categories);
  CategoriesElem * JsonStrToCategories(const std::string &json_str_categories);
private:
  void ConvertCategoriesToJson(Json::Value &json_value, const CategoriesElem* categories);
  void ConvertJsonToCategories(CategoriesElem* parent_category, const unsigned int &index, const Json::Value &json_value);
};

class CategoriesToJsonFileConverter final : public CategoriesToExtDbConverter {
public:
  CategoriesToJsonFileConverter(const std::string &json_db_filename, CategoriesToJsonConverter *categories_to_json_converter);
  void CategoriesToExtDb(CategoriesElem *categories) override;
  CategoriesElem * ExtDbToCategories() override;
private:
  std::string json_db_filename;
  CategoriesToJsonConverter *categories_to_json_converter;
};

#endif
