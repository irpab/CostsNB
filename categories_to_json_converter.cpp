
#include "categories_to_json_converter.h"


CategoriesToJsonFileConverter::CategoriesToJsonFileConverter(const std::string &json_db_filename_, CategoriesToJsonConverter *categories_to_json_converter_)
  : json_db_filename(json_db_filename_), categories_to_json_converter(categories_to_json_converter_)
{
}

void CategoriesToJsonFileConverter::CategoriesToExtDb(CategoriesElem *categories)
{
  std::ofstream json_db_file_stream;
  json_db_file_stream.open(json_db_filename);
  json_db_file_stream << categories_to_json_converter->CategoriesToJsonStr(categories) << std::endl;
  json_db_file_stream.close();
}

CategoriesElem * CategoriesToJsonFileConverter::ExtDbToCategories()
{
  if (utils::FileExists(json_db_filename)) {
    std::ifstream json_db_file_stream(json_db_filename, std::ifstream::binary);

    // determine size of file and pre-allocate string for efficiency
    json_db_file_stream.seekg(0, std::ios::end);
    size_t size = json_db_file_stream.tellg();
    std::string json_str(size, ' ');
    json_db_file_stream.seekg(0);

    json_db_file_stream.read(&json_str[0], size); 
    json_db_file_stream.close();

    return categories_to_json_converter->JsonStrToCategories(json_str);
  }
  // TODO: if file not exist return empty categories

  return nullptr;
}

// TODO: refactor
void CategoriesToJsonConverterJsoncppLib::ConvertCategoriesToJson(Json::Value &json_value, const CategoriesElem* categories)
{
  unsigned int sub_cat_num = 0;
  for (auto sub_category = categories->sub_categories.begin();
        sub_category != categories->sub_categories.end(); ++sub_category, ++sub_cat_num) {
    json_value["sub_categories"][sub_cat_num]["name"] = (*sub_category)->category_name;
    json_value["sub_categories"][sub_cat_num]["rating"] = (*sub_category)->rating;
    unsigned int k = 0;
    json_value["sub_categories"][sub_cat_num]["expenses"] = Json::Value(Json::arrayValue);
    for (auto expense = (*sub_category)->expenses.begin(); expense != (*sub_category)->expenses.end(); ++expense) {
      json_value["sub_categories"][sub_cat_num]["expenses"][k]["datetime"]["y"]  = expense->datetime.y;
      json_value["sub_categories"][sub_cat_num]["expenses"][k]["datetime"]["mn"] = expense->datetime.mn;
      json_value["sub_categories"][sub_cat_num]["expenses"][k]["datetime"]["d"]  = expense->datetime.d;
      json_value["sub_categories"][sub_cat_num]["expenses"][k]["datetime"]["h"]  = expense->datetime.h;
      json_value["sub_categories"][sub_cat_num]["expenses"][k]["datetime"]["m"]  = expense->datetime.m;
      json_value["sub_categories"][sub_cat_num]["expenses"][k]["datetime"]["s"]  = expense->datetime.s;
      json_value["sub_categories"][sub_cat_num]["expenses"][k]["cost"] = expense->cost;
      json_value["sub_categories"][sub_cat_num]["expenses"][k]["info"] = expense->info;
      k++;
    }
    Json::Value sub_categories;
    ConvertCategoriesToJson(sub_categories, *sub_category);
    if (sub_categories.isNull())
      json_value["sub_categories"][sub_cat_num]["sub_categories"] = Json::Value(Json::arrayValue);
    else
      json_value["sub_categories"][sub_cat_num]["sub_categories"] = sub_categories["sub_categories"];
  }
}

// TODO: refactor
void CategoriesToJsonConverterJsoncppLib::ConvertJsonToCategories(CategoriesElem* parent_category, const unsigned int &index, const Json::Value &json_value)
{
  if (index == json_value.size())
    return;
  std::string current_cat = json_value[index]["name"].asString();
  unsigned int current_cat_rating = json_value[index]["rating"].asInt();
  Json::Value sub_categories = json_value[index]["sub_categories"];
  CategoriesElem* new_sub_category = new CategoriesElem(current_cat, parent_category, current_cat_rating);
  for (unsigned int expense_num = 0; expense_num < json_value[index]["expenses"].size(); expense_num++) {
    Json::Value jsonDatetime = json_value[index]["expenses"][expense_num]["datetime"];
    auto y  = jsonDatetime["y"].asInt();
    auto mn = jsonDatetime["mn"].asInt();
    auto d  = jsonDatetime["d"].asInt();
    auto h  = jsonDatetime["h"].asInt();
    auto m  = jsonDatetime["m"].asInt();
    auto s  = jsonDatetime["s"].asInt();
    Datetime datetime(y, mn, d, h, m, s);
    Json::Value json_info = json_value[index]["expenses"][expense_num]["info"];
    std::string info = json_info.asString();
    new_sub_category->expenses.push_back(ExpenseElem(datetime, json_value[index]["expenses"][expense_num]["cost"].asInt(), info));
  }
  parent_category->sub_categories.push_back(new_sub_category);
  ConvertJsonToCategories(new_sub_category, 0, sub_categories);
  ConvertJsonToCategories(parent_category, index + 1, json_value);
}

// TODO: move semantics
std::string CategoriesToJsonConverterJsoncppLib::CategoriesToJsonStr(CategoriesElem *root_category)
{
  Json::Value json_categories;
  json_categories["version"] = SUPPORTED_DB_VERSION;
  CategoriesElem *dummy_root_category = new CategoriesElem("Dummy Root Category", nullptr);
  dummy_root_category->sub_categories.push_back(root_category);

  ConvertCategoriesToJson(json_categories, dummy_root_category);
  dummy_root_category->sub_categories.clear();
  delete dummy_root_category;
  return json_categories.toStyledString();
}

CategoriesElem * CategoriesToJsonConverterJsoncppLib::JsonStrToCategories(const std::string &json_str_categories)
{
  CategoriesElem* dummy_root_category = new CategoriesElem("Dummy Root Category", nullptr);
  std::istringstream json_str_categories_stream(json_str_categories);
  Json::Value json_categories;

  json_str_categories_stream >> json_categories;
  ConvertJsonToCategories(dummy_root_category, 0, json_categories["sub_categories"]);

  CategoriesElem* root_category = dummy_root_category->sub_categories.front();
  dummy_root_category->sub_categories.clear();
  delete dummy_root_category;
  root_category->parent_category = nullptr;

  return root_category;
}
