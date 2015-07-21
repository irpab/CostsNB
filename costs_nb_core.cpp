#include <fstream>
#include <string>
#include <iostream>

//#include <QDebug>

#include "costs_nb_core.h"
#include "costsnb_tcp_transport.h"

// TODO:
// -API should return success+result/error+reason values
// -add _internal functions for all APIs to cover with UT

/////////////////////////////
// internal functions
/////////////////////////////
std::string intMonth2str(const unsigned int month)
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

unsigned int strMonth2int(const std::string &month)
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

struct tm * now()
{
    time_t t = time(0);
    return localtime( & t );
}

bool compare_expenses_by_date(const Expense_elem &e1, const Expense_elem &e2)
{
    return e1.datetime > e2.datetime;
}

bool compare_categories_by_rating(const Categories_elem* l, const Categories_elem* r)
{
    return l->rating > r->rating;
}

bool fileExists(const std::string& fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

void Costs_nb_core::Convert_json_to_categories(Categories_elem* parentCategory, const unsigned int &index, const Json::Value &jsonCategories)
{
  if (index == jsonCategories.size())
    return;
  std::string currentCat = jsonCategories[index]["name"].asString();
  unsigned int currentCatRating = jsonCategories[index]["rating"].asInt();
  Json::Value subCategories = jsonCategories[index]["sub_categories"];
  Categories_elem* newSubCategory = new Categories_elem(currentCat, parentCategory, currentCatRating);
  for (unsigned int i = 0; i < jsonCategories[index]["expenses"].size(); i++) {
    Expense_elem::Datetime datetime;
    Json::Value jsonDatetime = jsonCategories[index]["expenses"][i]["datetime"];
    if (jsonDatetime.isString()) {
        // parse string "y-Mn-d h:m" to datetime
        std::stringstream ss(jsonDatetime.asString());
        std::string item;
        std::getline(ss, item, '-');
//        datetime.y  = std::stoi(item); std::getline(ss, item, '-');
//        datetime.mn = strMonth2int(item); std::getline(ss, item, ' ');
//        datetime.d  = std::stoi(item); std::getline(ss, item, ':');
//        datetime.h  = std::stoi(item); std::getline(ss, item);
//        datetime.m  = std::stoi(item);
        std::istringstream(item) >> datetime.y; std::getline(ss, item, '-');
        datetime.mn = strMonth2int(item); std::getline(ss, item, ' ');
        std::istringstream(item) >> datetime.d; std::getline(ss, item, ':');
        std::istringstream(item) >> datetime.h; std::getline(ss, item);
        std::istringstream(item) >> datetime.m;
        datetime.s  = 0;
    } else if (jsonDatetime.isObject()) {
        datetime.y  = jsonDatetime["y"].asInt();
        datetime.mn = jsonDatetime["mn"].asInt();
        datetime.d  = jsonDatetime["d"].asInt();
        datetime.h  = jsonDatetime["h"].asInt();
        datetime.m  = jsonDatetime["m"].asInt();
        datetime.s  = jsonDatetime["s"].asInt();
    }
    Json::Value jsonInfo = jsonCategories[index]["expenses"][i]["info"];
    std::string info;
    if (jsonInfo.isNull()) {
        info = "";
    } else {
        info = jsonInfo.asString();
    }
    newSubCategory->expenses.push_back(Expense_elem(datetime, jsonCategories[index]["expenses"][i]["cost"].asInt(), info));
  }
  parentCategory->subCategories.push_back(newSubCategory);
  Convert_json_to_categories(newSubCategory, 0, subCategories);
  Convert_json_to_categories(parentCategory, index+1, jsonCategories);
}

Categories_elem* Costs_nb_core::Read_categories_from_db(const std::string &dbFile)
{
    Categories_elem* rootCategory = new Categories_elem("Root Category", nullptr);
    Json::Value rootJson;

    if (fileExists(dbFile)) {
        std::ifstream dbFileStream(dbFile, std::ifstream::binary);
        dbFileStream >> rootJson;
        dbFileStream.close();

        std::string dbVersion = rootJson["version"].asString();
        Convert_json_to_categories(rootCategory, 0, rootJson["sub_categories"]);
    }

    return rootCategory;
}

void Costs_nb_core::Convert_categories_to_json(Json::Value &rootJson, const Categories_elem* categories)
{
    unsigned int j = 0;
    for (auto i = categories->subCategories.begin();
            i != categories->subCategories.end(); ++i, ++j) {
        rootJson["sub_categories"][j]["name"] = (*i)->categoryName;
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
        Convert_categories_to_json(Sub_categories, *i);
        if (Sub_categories.isNull())
            rootJson["sub_categories"][j]["sub_categories"] = Json::Value(Json::arrayValue);
        else
            rootJson["sub_categories"][j]["sub_categories"] = Sub_categories["sub_categories"];
    }
}

void Costs_nb_core::Write_categories_to_db(const Categories_elem* rootCategory, const std::string &dbFileName)
{
    std::ofstream dbFile;
    dbFile.open(dbFileName);
    Json::Value rootJson;
    rootJson["version"] = SUPPORTED_DB_VERSION;
    Convert_categories_to_json(rootJson, rootCategory);
    dbFile << rootJson << std::endl;
    dbFile.close();
}

void Costs_nb_core::Sync_db_with_server()
{
    // read config and get server ip, port, last sync time
    if (!fileExists(cfgFileName))
        return;

    std::ifstream cfgFileStream(cfgFileName, std::ifstream::binary);
    std::string ip;
    std::string portStr;
    int port;
    std::string last_time;
    cfgFileStream >> ip;
    cfgFileStream >> portStr;
    std::istringstream(portStr) >> port;
    cfgFileStream >> last_time;
    cfgFileStream.close();

    struct tm * t = now();
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
      Convert_categories_to_json(rootJson, categories);

      // update last sync time
      std::ofstream cfgFileStreamOut;
      cfgFileStreamOut.open(cfgFileName);
      cfgFileStreamOut << ip << std::endl;
      cfgFileStreamOut << port << std::endl;
      cfgFileStreamOut << t->tm_mday << std::endl;
      cfgFileStreamOut.close();

      Send_string(tcpTransport, rootJson.toStyledString());
    }
    catch (std::exception &e) {
    }
}

bool ValidateCategoryName(const std::string &newCategory)
{
    if (newCategory.size() == 0)
        return false;
    return true;
}

bool SubCategoriesContainCategory(const Categories_elem* categories, const std::string &categoryName)
{
    for (auto i = categories->subCategories.begin(); i != categories->subCategories.end(); ++i)
    {
        if (0 == (*i)->categoryName.compare(categoryName))
        {
            return true;
        }
    }
    return false;
}

std::string AddDisplaySubCategoriesPrefix(const Categories_elem* category)
{
    std::string hasSubcategoriesPrefix;
    if (category->subCategories.empty())
        hasSubcategoriesPrefix = "";
    else
        hasSubcategoriesPrefix = "> ";
    return hasSubcategoriesPrefix + category->categoryName;
}

std::string RemoveDisplaySubCategoriesPrefix(const std::string categoryName)
{
    if (categoryName.length() > 2 && !categoryName.compare(0,2,"> "))
        return categoryName.substr(2);
    else
        return categoryName;
}

Categories_elem* GetSubCategoryByName(const Categories_elem* category, const std::string &categoryName)
{
    for (auto i = category->subCategories.begin(); i != category->subCategories.end(); ++i)
    {
        if (0 == (*i)->categoryName.compare(categoryName))
        {
            return *i;
        }
    }
    return nullptr;
}

bool AddSubCategory(Categories_elem* category, const std::string &newCategoryName)
{
    if (!ValidateCategoryName(newCategoryName))
        return false;

    if (SubCategoriesContainCategory(category, newCategoryName))
        return false;

    category->subCategories.push_back(new Categories_elem(newCategoryName, category));
    return true;
}

void GetAllNestedExpenses(std::list<Expense_elem> &expenses, const Categories_elem* category)
{
    for (auto i = category->expenses.begin(); i != category->expenses.end(); ++i) {
        expenses.push_back(*i);
    }
    for (auto i = category->subCategories.begin(); i != category->subCategories.end(); ++i) {
        GetAllNestedExpenses(expenses, *i);
    }
}

bool RenameCategory_internal(Categories_elem* categories, const std::string &oldName0, const std::string &newName)
{
    std::string oldName = RemoveDisplaySubCategoriesPrefix(oldName0);

    if (!ValidateCategoryName(newName))
        return false;

    Categories_elem* category = GetSubCategoryByName(categories, oldName);
    if (category == nullptr)
        return false;

    if (SubCategoriesContainCategory(categories, newName))
        return false;

    category->categoryName = newName;
    return true;
}

std::list<std::string> GetExpenses_internal(Categories_elem* categories, const std::string &selectedCategory0)
{
    std::list<std::string> expenses;
    std::string selectedCategory = RemoveDisplaySubCategoriesPrefix(selectedCategory0);

    Categories_elem* category = GetSubCategoryByName(categories, selectedCategory);
    if (category != nullptr) {
        for (auto i = category->expenses.begin(); i != category->expenses.end(); ++i) {
            expenses.push_back(i->toStr());
        }
    }
    return expenses;
}

std::list<std::string> GetAllExpenses_internal(Categories_elem* categories, const std::string &selectedCategory0)
{
    std::list<std::string> expensesStr;
    std::string selectedCategory = RemoveDisplaySubCategoriesPrefix(selectedCategory0);
    std::cout << "GetAllExpenses_internal: selectedCategory = " << selectedCategory << std::endl;

    Categories_elem* category = GetSubCategoryByName(categories, selectedCategory);
    if (category != nullptr) {
        std::list<Expense_elem> expenses;
        GetAllNestedExpenses(expenses, category);
        expenses.sort(compare_expenses_by_date);
        for (auto i = expenses.begin(); i != expenses.end(); ++i) {
            expensesStr.push_back(i->toStr());
        }
    }
    return expensesStr;
}

std::string tm2str(const struct tm * t)
{
    std::stringstream ss;
    ss << (t->tm_year + 1900) << '-' << intMonth2str(t->tm_mon + 1) << '-'
        << std::setw(2) << std::setfill('0') << t->tm_mday
        << " " << t->tm_hour << ":" << std::setw(2) << std::setfill('0') << t->tm_min;
    return ss.str();
}

std::string now2str()
{
    struct tm * t = now();
    return tm2str(t);
}


Costs_nb_core::Costs_nb_core(const std::string &dbFileName0, const std::string &cfgFileName0)
{
    dbFileName = dbFileName0;
    cfgFileName = cfgFileName0;
    categories = Read_categories_from_db(dbFileName);
    Sync_db_with_server();
}

Costs_nb_core::~Costs_nb_core()
{
    Categories_elem* rootCategory = categories;
    while (rootCategory->parentCategory != nullptr)
        rootCategory = rootCategory->parentCategory;
    Write_categories_to_db(rootCategory, dbFileName);
}

void Buy_internal(Categories_elem* categories, const std::string &selectedCategory, const unsigned int &cost, const std::string &info)
{
    Categories_elem* category = nullptr;
    if (selectedCategory.empty())
        category = categories;
    else
        category = GetSubCategoryByName(categories, selectedCategory);

    if (category == nullptr)
        return;

    category->expenses.push_back(Expense_elem(now(), cost, info));

    while (category != nullptr) {
        category->rating++;
        category->subCategories.sort(compare_categories_by_rating);
        category = category->parentCategory;
    }
}

void Buy_internal(Categories_elem* categories, const std::string &selectedCategory, const unsigned int &cost)
{
    Buy_internal(categories, selectedCategory, cost, "");
}

/////////////////////////////
// API
/////////////////////////////
std::tuple<std::list<std::string>, std::string> Costs_nb_core::GetCurrentCategories()
{
    std::list<std::string> categoriesNames;
    for (auto i = categories->subCategories.begin(); i != categories->subCategories.end(); ++i)
    {
        std::string displayCategoryName = AddDisplaySubCategoriesPrefix(*i);
        categoriesNames.push_back(displayCategoryName);
    }
    return std::make_tuple(categoriesNames, categories->categoryName);
}

bool Costs_nb_core::CategorySelected(const std::string &selectedCategoryName0)
{
    std::string selectedCategoryName = RemoveDisplaySubCategoriesPrefix(selectedCategoryName0);

    Categories_elem* selectedCategory = GetSubCategoryByName(categories, selectedCategoryName);
    if (selectedCategory == nullptr)
        return false;

    if (selectedCategory->subCategories.empty())
        return true;

    categories = selectedCategory;
    return false;
}

void Costs_nb_core::CategoryBack()
{
    if (categories->parentCategory != nullptr)
    {
        categories = categories->parentCategory;
    }
}

bool Costs_nb_core::CategoryAdd(const std::string &newCategoryName)
{
    return AddSubCategory(categories, newCategoryName);
}

bool Costs_nb_core::CategoryAddSub(const std::string &parentCategoryName0, const std::string &newCategoryName)
{
    std::string parentCategoryName = RemoveDisplaySubCategoriesPrefix(parentCategoryName0);

    Categories_elem* parentCategory = GetSubCategoryByName(categories, parentCategoryName);
    if (parentCategory == nullptr)
        return false;

    if (!AddSubCategory(parentCategory, newCategoryName))
        return false;

    categories = parentCategory;
    return true;
}

bool Costs_nb_core::RemoveCategory(const std::string &removingCategoryName0)
{
    std::string removingCategoryName = RemoveDisplaySubCategoriesPrefix(removingCategoryName0);

    Categories_elem* category = GetSubCategoryByName(categories, removingCategoryName);
    if (category == nullptr)
        return false;

    std::list<Expense_elem> expenses;
    GetAllNestedExpenses(expenses, category);
    for (auto i = expenses.begin(); i != expenses.end(); ++i) {
        category->parentCategory->expenses.push_back(*i);
    }

    categories->subCategories.remove(category);
    if (categories->subCategories.empty())
        CategoryBack();
    return true;
}

bool Costs_nb_core::RenameCategory(const std::string &oldName, const std::string &newName)
{
    return RenameCategory_internal(categories, oldName, newName);
}

void Costs_nb_core::Buy(const std::string &selectedCategory, const unsigned int &cost, const std::string &info)
{
    Buy_internal(categories, selectedCategory, cost, info);
}

std::list<std::string> Costs_nb_core::GetExpenses(const std::string &selectedCategory0)
{
    return GetExpenses_internal(categories, selectedCategory0);
}

std::list<std::string> Costs_nb_core::GetAllExpenses(const std::string &selectedCategory0)
{
    return GetAllExpenses_internal(categories, selectedCategory0);
}
