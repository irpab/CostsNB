#ifndef COSTS_NB_CORE_H
#define COSTS_NB_CORE_H

#include <list>
#include <string>
#include <tuple>
#include <sstream>

#include "json/json-forwards.h"
#include "json/json.h"

#define SUPPORTED_DB_VERSION 1

namespace workaround // workaround
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

class Expense_elem
{
public:
    Expense_elem(std::string datetime0, unsigned int cost0) :
        datetime(datetime0), cost(cost0)
    {}

    std::string toStr() const
    {
        datetime + "   " + workaround::to_string(cost);
    }

    std::string datetime;
    unsigned int cost;
};


class Categories_elem
{
public:
    Categories_elem(const std::string &categoryName0, Categories_elem* parentCategory0) :
        categoryName(categoryName0), rating(0), parentCategory(parentCategory0)
    {
    }
    Categories_elem(const std::string &categoryName0, Categories_elem* parentCategory0, unsigned int rating0) :
        categoryName(categoryName0), rating(rating0), parentCategory(parentCategory0)
    {
    }

    std::string categoryName;
    unsigned int rating;
    Categories_elem* parentCategory;
    std::list<Categories_elem*> subCategories;
    std::list<Expense_elem> expenses;
};


class Costs_nb_core
{
    Categories_elem *categories;
    std::string dbFileName;

    Categories_elem* Read_categories_from_db(const std::string &dbFile);
    void Convert_json_to_categories(Categories_elem* parentCategory, const unsigned int &index, const Json::Value &jsonCategories);
    void Convert_categories_to_json(Json::Value &rootJson, const Categories_elem* categories);
    void Write_categories_to_db(const Categories_elem* rootCategory, const std::string &dbFileName);
public:
    Costs_nb_core(const std::string &dbFile);
    ~Costs_nb_core();

    std::tuple<std::list<std::string>, std::string> GetCurrentCategories();
    bool CategorySelected(const std::string &selectedCategory);
    bool RemoveCategory(const std::string &selectedCategory);
    bool RenameCategory(const std::string &oldName, const std::string &newName);
    void CategoryBack();
    bool CategoryAdd(const std::string &newCategory);
    bool CategoryAddSub(const std::string &parentCategory, const std::string &newCategory);
    void Buy(const std::string &selectedCategory, const unsigned int &cost);
    std::list<std::string> GetExpenses(const std::string &selectedCategory);
    std::list<std::string> GetAllExpenses(const std::string &selectedCategory);
};

#endif // COSTS_NB_CORE_H
