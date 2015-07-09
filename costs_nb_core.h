#ifndef COSTS_NB_CORE_H
#define COSTS_NB_CORE_H

#include <list>
#include <string>
#include <tuple>
#include <sstream>
#include <ctime>
#include <iomanip>

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

std::string intMonth2str(unsigned int month);

struct Expense_elem
{
    struct Datetime
    {
        Datetime()
        {
            y  = 2000;
            mn = 1;
            d  = 1;
            h  = 0;
            m  = 0;
            s  = 0;
        }
        Datetime(unsigned short y0, unsigned short mn0, unsigned short d0,
                 unsigned short h0, unsigned short m0, unsigned short s0) :
            y(y0), mn(mn0), d(d0), h(h0), m(m0), s(s0)
        {}
        Datetime(const struct tm * t)
        {
            y = t->tm_year + 1900;
            mn = t->tm_mon + 1;
            d = t->tm_mday;
            h = t->tm_hour;
            m = t->tm_min;
            s = t->tm_sec;
        }

        bool operator>(const Datetime &r) const {
            if (y > r.y) return true;
            if (y < r.y) return false;

            if (mn > r.mn) return true;
            if (mn < r.mn) return false;

            if (d > r.d) return true;
            if (d < r.d) return false;

            if (h > r.h) return true;
            if (h < r.h) return false;

            if (m > r.m) return true;
            if (m < r.m) return false;

            if (s > r.s) return true;
            if (s < r.s) return false;

            return false;
        }

        std::string toStr() const
        {
            std::stringstream ss;
            ss << y << '-' << intMonth2str(mn) << '-'
                << std::setw(2) << std::setfill('0') << d
                << " "<< std::setw(2) << std::setfill('0') << h
                << ":" << std::setw(2) << std::setfill('0') << m;
            return ss.str();
        }

        unsigned short y, mn, d, h, m, s;
    };

    Expense_elem(const struct tm * datetime0, unsigned int cost0) :
        datetime(datetime0), cost(cost0), info("")
    {}

    Expense_elem(const Datetime datetime0, unsigned int cost0) :
        datetime(datetime0), cost(cost0), info("")
    {}

    Expense_elem(const Datetime datetime0, unsigned int cost0, const std::string &info0) :
        datetime(datetime0), cost(cost0), info(info0)
    {}

    Expense_elem(const struct tm * datetime0, unsigned int cost0, const std::string &info0) :
        datetime(datetime0), cost(cost0), info(info0)
    {}

    std::string toStr() const
    {
        return datetime.toStr() + "   " + workaround::to_string(cost);
    }

    Datetime datetime;
    unsigned int cost;
    std::string info;
};


struct Categories_elem
{
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
    void Buy(const std::string &selectedCategory, const unsigned int &cost, const std::string &info);
    std::list<std::string> GetExpenses(const std::string &selectedCategory);
    std::list<std::string> GetAllExpenses(const std::string &selectedCategory);
};

#endif // COSTS_NB_CORE_H
