#include <fstream>
#include <iomanip>
#include <ctime>

//#include <QDebug>

#include "costs_nb_core.h"

/////////////////////////////
// internal functions
/////////////////////////////
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
    newSubCategory->expenses.push_back(Expense_elem(jsonCategories[index]["expenses"][i]["datetime"].asString(), jsonCategories[index]["expenses"][i]["cost"].asInt()));
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
            rootJson["sub_categories"][j]["expenses"][k]["datetime"] = expense->datetime;
            rootJson["sub_categories"][j]["expenses"][k]["cost"] = expense->cost;
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

std::string intMonth2str(unsigned int month)
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

std::string now2str()
{
    time_t t = time(0);
    struct tm * now = localtime( & t );
    std::stringstream ss;
    ss << (now->tm_year + 1900) << '-' << intMonth2str(now->tm_mon + 1) << '-'
        << std::setw(2) << std::setfill('0') << now->tm_mday
        << " " << now->tm_hour << ":" << std::setw(2) << std::setfill('0') << now->tm_min;
    return ss.str();
}


Costs_nb_core::Costs_nb_core(const std::string &dbFileName0)
{
    dbFileName = dbFileName0;
    categories = Read_categories_from_db(dbFileName);
}

Costs_nb_core::~Costs_nb_core()
{
    Categories_elem* rootCategory = categories;
    while (rootCategory->parentCategory != nullptr)
        rootCategory = rootCategory->parentCategory;
    Write_categories_to_db(rootCategory, dbFileName);
}


/////////////////////////////
// API
/////////////////////////////
bool compare_categories(const Categories_elem* l, const Categories_elem* r)
{
    return l->rating > r->rating;
}

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

void GetAllNestedExpenses(std::list<Expense_elem> &expenses, const Categories_elem* category)
{
    for (auto i = category->expenses.begin(); i != category->expenses.end(); ++i) {
        expenses.push_back(*i);
    }
    for (auto i = category->subCategories.begin(); i != category->subCategories.end(); ++i) {
        GetAllNestedExpenses(expenses, *i);
    }
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

void Costs_nb_core::Buy(const std::string &selectedCategory, const unsigned int &cost)
{
    Categories_elem* category = nullptr;
    if (selectedCategory.empty())
        category = categories;
    else
        category = GetSubCategoryByName(categories, selectedCategory);

    if (category == nullptr)
        return;

    category->rating++;
    category->expenses.push_back(Expense_elem(now2str(), cost));

    if (category->parentCategory != nullptr)
        category->parentCategory->subCategories.sort(compare_categories);
}

std::list<std::string> Costs_nb_core::GetExpenses(const std::string &selectedCategory0)
{
    std::list<std::string> expenses;
    std::string selectedCategory = RemoveDisplaySubCategoriesPrefix(selectedCategory0);

    Categories_elem* category = GetSubCategoryByName(categories, selectedCategory);
    if (category != nullptr) {
        for (auto i = category->expenses.begin(); i != category->expenses.end(); ++i) {
            expenses.push_back(i->datetime + "   " + workaround::to_string(i->cost));
        }
    }
    return expenses;
}
