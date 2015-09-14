#include <fstream>
#include <string>
#include <iostream>

//#include <QDebug>

#include "costs_nb_core.h"
#include "costsnb_tcp_transport.h"

//////// TODO: refactor all below
void CostsNbCore::ConvertJsonToCategories(CategoriesElem* parent_category, const unsigned int &index, const Json::Value &jsonCategories)
{
  if (index == jsonCategories.size())
    return;
  std::string currentCat = jsonCategories[index]["name"].asString();
  unsigned int currentCatRating = jsonCategories[index]["rating"].asInt();
  Json::Value subCategories = jsonCategories[index]["sub_categories"];
  CategoriesElem* newSubCategory = new CategoriesElem(currentCat, parent_category, currentCatRating);
  for (unsigned int i = 0; i < jsonCategories[index]["expenses"].size(); i++) {
    ExpenseElem::Datetime datetime;
    Json::Value jsonDatetime = jsonCategories[index]["expenses"][i]["datetime"];
    datetime.y  = jsonDatetime["y"].asInt();
    datetime.mn = jsonDatetime["mn"].asInt();
    datetime.d  = jsonDatetime["d"].asInt();
    datetime.h  = jsonDatetime["h"].asInt();
    datetime.m  = jsonDatetime["m"].asInt();
    datetime.s  = jsonDatetime["s"].asInt();
    Json::Value jsonInfo = jsonCategories[index]["expenses"][i]["info"];
    std::string info = jsonInfo.asString();
    newSubCategory->expenses.push_back(ExpenseElem(datetime, jsonCategories[index]["expenses"][i]["cost"].asInt(), info));
  }
  parent_category->sub_categories.push_back(newSubCategory);
  ConvertJsonToCategories(newSubCategory, 0, subCategories);
  ConvertJsonToCategories(parent_category, index+1, jsonCategories);
}

CategoriesElem* CostsNbCore::ReadCategoriesFromDb(const std::string &dbFile)
{
    CategoriesElem* rootCategory = new CategoriesElem("Root Category", nullptr);
    Json::Value rootJson;

    if (utils::FileExists(dbFile)) {
        std::ifstream dbFileStream(dbFile, std::ifstream::binary);
        dbFileStream >> rootJson;
        dbFileStream.close();

        std::string dbVersion = rootJson["version"].asString();
        ConvertJsonToCategories(rootCategory, 0, rootJson["sub_categories"]);
    }

    return rootCategory;
}

void CostsNbCore::ConvertCategoriesToJson(Json::Value &rootJson, const CategoriesElem* categories)
{
    unsigned int j = 0;
    for (auto i = categories->sub_categories.begin();
            i != categories->sub_categories.end(); ++i, ++j) {
        rootJson["sub_categories"][j]["name"] = (*i)->category_name;
        rootJson["sub_categories"][j]["rating"] = (*i)->rating;
        unsigned int k = 0;
        rootJson["sub_categories"][j]["expenses"] = Json::Value(Json::arrayValue);
        for (auto expense = (*i)->expenses.begin(); expense != (*i)->expenses.end(); ++expense) {
            rootJson["sub_categories"][j]["expenses"][k]["datetime"]["y"]  = expense->datetime.y;
            rootJson["sub_categories"][j]["expenses"][k]["datetime"]["mn"] = expense->datetime.mn;
            rootJson["sub_categories"][j]["expenses"][k]["datetime"]["d"]  = expense->datetime.d;
            rootJson["sub_categories"][j]["expenses"][k]["datetime"]["h"]  = expense->datetime.h;
            rootJson["sub_categories"][j]["expenses"][k]["datetime"]["m"]  = expense->datetime.m;
            rootJson["sub_categories"][j]["expenses"][k]["datetime"]["s"]  = expense->datetime.s;
            rootJson["sub_categories"][j]["expenses"][k]["cost"] = expense->cost;
            rootJson["sub_categories"][j]["expenses"][k]["info"] = expense->info;
            k++;
        }
        Json::Value Sub_categories;
        ConvertCategoriesToJson(Sub_categories, *i);
        if (Sub_categories.isNull())
            rootJson["sub_categories"][j]["sub_categories"] = Json::Value(Json::arrayValue);
        else
            rootJson["sub_categories"][j]["sub_categories"] = Sub_categories["sub_categories"];
    }
}

void CostsNbCore::WriteCategoriesToDb(const CategoriesElem* rootCategory, const std::string &dbFileName)
{
    std::ofstream dbFile;
    dbFile.open(dbFileName);
    Json::Value rootJson;
    rootJson["version"] = SUPPORTED_DB_VERSION;
    ConvertCategoriesToJson(rootJson, rootCategory);
    dbFile << rootJson << std::endl;
    dbFile.close();
}

void CostsNbCore::SyncDbWithServer()
{
    // read config and get server ip, port, last sync time
    if (!utils::FileExists(cfg_file_name))
        return;

    std::ifstream cfgFileStream(cfg_file_name, std::ifstream::binary);
    std::string ip;
    std::string portStr;
    int port;
    std::string last_time;
    cfgFileStream >> ip;
    cfgFileStream >> portStr;
    std::istringstream(portStr) >> port;
    cfgFileStream >> last_time;
    cfgFileStream.close();

    struct tm * t = utils::Now();
    std::stringstream ss;
    ss << t->tm_mday;

    if (!last_time.empty() && !last_time.compare(ss.str()))
        return;

    // try to establish connection and do handshake
    try {
      CostsNBTcpTransport tcpTransport(ip, port);
      Handshake(tcpTransport);
  
      // convert categories to json and send them to server
      Json::Value rootJson;
      rootJson["version"] = SUPPORTED_DB_VERSION;
      ConvertCategoriesToJson(rootJson, categories);

      // update last sync time
      std::ofstream cfgFileStreamOut;
      cfgFileStreamOut.open(cfg_file_name);
      cfgFileStreamOut << ip << std::endl;
      cfgFileStreamOut << port << std::endl;
      cfgFileStreamOut << t->tm_mday << std::endl;
      cfgFileStreamOut.close();

      Send_string(tcpTransport, rootJson.toStyledString());
    }
    catch (std::exception &e) {
    }
}

//////// TODO: refactor all above

bool CompareExpensesByDate(const ExpenseElem &e1, const ExpenseElem &e2)
{
    return e1.datetime > e2.datetime;
}

bool CompareCategoriesByRating(const CategoriesElem* e1, const CategoriesElem* e2)
{
    return e1->rating > e2->rating;
}

bool ValidateCategoryName(const std::string &category_name)
{
    if (category_name.size() == 0)
        return false;
    if (category_name.length() >= 2 && !category_name.compare(0,2,"> "))
        return false;
    return true;
}

bool SubCategoriesContainCategory(const CategoriesElem* categories, const std::string &category_name)
{
    for (auto category = categories->sub_categories.begin(); category != categories->sub_categories.end(); ++category)
    {
        if (0 == (*category)->category_name.compare(category_name))
        {
            return true;
        }
    }
    return false;
}

// TODO: avoid text prefixes, use objects
std::string AddDisplaySubCategoriesPrefix(const CategoriesElem* category)
{
    std::string has_subcategories_prefix;
    if (category->sub_categories.empty())
        has_subcategories_prefix = "";
    else
        has_subcategories_prefix = "> ";
    return has_subcategories_prefix + category->category_name;
}

std::string RemoveDisplaySubCategoriesPrefix(const std::string category_name)
{
    if (category_name.length() > 2 && !category_name.compare(0,2,"> "))
        return category_name.substr(2);
    return category_name;
}

CategoriesElem* GetSubCategoryByName(const CategoriesElem* category, const std::string &category_name)
{
    for (auto sub_category = category->sub_categories.begin(); sub_category != category->sub_categories.end(); ++sub_category)
    {
        if (0 == (*sub_category)->category_name.compare(category_name))
        {
            return *sub_category;
        }
    }
    return nullptr;
}

bool AddSubCategory(CategoriesElem* category, const std::string &new_category_name)
{
    if (!ValidateCategoryName(new_category_name))
        return false;

    if (SubCategoriesContainCategory(category, new_category_name))
        return false;

    category->sub_categories.push_back(new CategoriesElem(new_category_name, category));
    return true;
}

void GetAllNestedExpenses(std::list<ExpenseElem> &expenses, const CategoriesElem* category)
{
    for (auto expense = category->expenses.begin(); expense != category->expenses.end(); ++expense) {
        expenses.push_back(*expense);
    }
    for (auto sub_category = category->sub_categories.begin(); sub_category != category->sub_categories.end(); ++sub_category) {
        GetAllNestedExpenses(expenses, *sub_category);
    }
}

bool RenameCategory(CategoriesElem* categories, const std::string &old_name_, const std::string &new_name)
{
    std::string old_name = RemoveDisplaySubCategoriesPrefix(old_name_);

    if (!ValidateCategoryName(new_name))
        return false;

    CategoriesElem* category = GetSubCategoryByName(categories, old_name);
    if (category == nullptr)
        return false;

    if (SubCategoriesContainCategory(categories, new_name))
        return false;

    category->category_name = new_name;
    return true;
}

std::list<std::string> GetExpenses(CategoriesElem* categories, const std::string &selected_category_)
{
    std::list<std::string> expenses;
    std::string selected_category = RemoveDisplaySubCategoriesPrefix(selected_category_);

    CategoriesElem* category = GetSubCategoryByName(categories, selected_category);
    if (category != nullptr) {
        for (auto expense = category->expenses.begin(); expense != category->expenses.end(); ++expense) {
            expenses.push_back(expense->ToStr());
        }
    }
    return expenses;
}

std::list<std::string> GetAllExpenses(CategoriesElem* categories, const std::string &selected_category_)
{
    std::list<std::string> expenses_str;
    std::string selected_category = RemoveDisplaySubCategoriesPrefix(selected_category_);

    CategoriesElem* category = GetSubCategoryByName(categories, selected_category);
    if (category != nullptr) {
        std::list<ExpenseElem> expenses;
        GetAllNestedExpenses(expenses, category);
        expenses.sort(CompareExpensesByDate);
        for (auto expense = expenses.begin(); expense != expenses.end(); ++expense) {
            expenses_str.push_back(expense->ToStr());
        }
    }
    return expenses_str;
}

void Buy(CategoriesElem* categories, const std::string &selected_category, const unsigned int cost, const std::string &info)
{
    CategoriesElem* category = nullptr;
    if (selected_category.empty())
        category = categories;
    else
        category = GetSubCategoryByName(categories, selected_category);

    if (category == nullptr)
        return;

    category->expenses.push_back(ExpenseElem(utils::Now(), cost, info));

    while (category != nullptr) {
        category->rating++;
        category->sub_categories.sort(CompareCategoriesByRating);
        category = category->parent_category;
    }
}

/////////////////////////////
// API
/////////////////////////////

CostsNbCore::CostsNbCore(const std::string &db_file_name_, const std::string &cfg_file_name_)
{
    db_file_name = db_file_name_;
    cfg_file_name = cfg_file_name_;
    categories = ReadCategoriesFromDb(db_file_name);
    SyncDbWithServer();
}

CostsNbCore::~CostsNbCore()
{
    CategoriesElem* root_category = categories;
    while (root_category->parent_category != nullptr)
        root_category = root_category->parent_category;
    WriteCategoriesToDb(root_category, db_file_name);
}

std::tuple<std::list<std::string>, std::string> CostsNbCore::GetCurrentCategories()
{
    std::list<std::string> categories_names;
    for (auto category = categories->sub_categories.begin(); category != categories->sub_categories.end(); ++category)
    {
        std::string display_category_name = AddDisplaySubCategoriesPrefix(*category);
        categories_names.push_back(display_category_name);
    }
    return std::make_tuple(categories_names, categories->category_name);
}

bool CostsNbCore::CategorySelected(const std::string &selected_category_name_)
{
    std::string selected_category_name = RemoveDisplaySubCategoriesPrefix(selected_category_name_);

    CategoriesElem* selected_category = GetSubCategoryByName(categories, selected_category_name);
    if (selected_category == nullptr)
        return false;

    if (selected_category->sub_categories.empty())
        return true;

    categories = selected_category;
    return false;
}

void CostsNbCore::CategoryBack()
{
    if (categories->parent_category != nullptr)
    {
        categories = categories->parent_category;
    }
}

bool CostsNbCore::CategoryAdd(const std::string &new_category_name)
{
    return AddSubCategory(categories, new_category_name);
}

bool CostsNbCore::CategoryAddSub(const std::string &parent_category_name_, const std::string &new_category_name)
{
    std::string parent_category_name = RemoveDisplaySubCategoriesPrefix(parent_category_name_);

    CategoriesElem* parent_category = GetSubCategoryByName(categories, parent_category_name);
    if (parent_category == nullptr)
        return false;

    if (!AddSubCategory(parent_category, new_category_name))
        return false;

    categories = parent_category;
    return true;
}

bool CostsNbCore::RemoveCategory(const std::string &removing_category_name_)
{
    std::string removing_category_name = RemoveDisplaySubCategoriesPrefix(removing_category_name_);

    CategoriesElem* category = GetSubCategoryByName(categories, removing_category_name);
    if (category == nullptr)
        return false;

    std::list<ExpenseElem> expenses;
    GetAllNestedExpenses(expenses, category);
    for (auto expense = expenses.begin(); expense != expenses.end(); ++expense) {
        category->parent_category->expenses.push_back(*expense);
    }

    categories->sub_categories.remove(category);
    if (categories->sub_categories.empty())
        CategoryBack();
    return true;
}

bool CostsNbCore::RenameCategory(const std::string &old_name, const std::string &new_name)
{
    return ::RenameCategory(categories, old_name, new_name);
}

void CostsNbCore::Buy(const std::string &selected_category, const unsigned int &cost, const std::string &info)
{
    ::Buy(categories, selected_category, cost, info);
}

std::list<std::string> CostsNbCore::GetExpenses(const std::string &selected_category)
{
    return ::GetExpenses(categories, selected_category);
}

std::list<std::string> CostsNbCore::GetAllExpenses(const std::string &selected_category)
{
    return ::GetAllExpenses(categories, selected_category);
}
