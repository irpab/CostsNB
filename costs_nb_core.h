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

#include "utils.h"

#define SUPPORTED_DB_VERSION 1

struct ExpenseElem
{
    struct Datetime
    {
        Datetime(unsigned short y_, unsigned short mn_, unsigned short d_,
                 unsigned short h_, unsigned short m_, unsigned short s_) :
            y(y_), mn(mn_), d(d_), h(h_), m(m_), s(s_)
        {}
        Datetime(const struct tm * t)
        {
            y  = t->tm_year + 1900;
            mn = t->tm_mon + 1;
            d  = t->tm_mday;
            h  = t->tm_hour;
            m  = t->tm_min;
            s  = t->tm_sec;
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

        std::string ToStr() const
        {
            std::stringstream ss;
            ss << y << '-' << utils::MonthNumToStr(mn) << '-'
              << std::setw(2) << std::setfill('0') << d << " "
              << std::setw(2) << std::setfill('0') << h << ":"
              << std::setw(2) << std::setfill('0') << m;
            return ss.str();
        }

        unsigned short y, mn, d, h, m, s;
    };

    ExpenseElem(const struct tm * datetime_, unsigned int cost_) :
        datetime(datetime_), cost(cost_), info("")
    {}

    ExpenseElem(const Datetime datetime_, unsigned int cost_) :
        datetime(datetime_), cost(cost_), info("")
    {}

    ExpenseElem(const Datetime datetime_, unsigned int cost_, const std::string &info_) :
        datetime(datetime_), cost(cost_), info(info_)
    {}

    ExpenseElem(const struct tm * datetime_, unsigned int cost_, const std::string &info_) :
        datetime(datetime_), cost(cost_), info(info_)
    {}

    std::string ToStr() const
    {
        return datetime.ToStr() + "   " + utils::to_string(cost);
    }

    Datetime datetime;
    unsigned int cost;
    std::string info;
};


struct CategoriesElem
{
    CategoriesElem(const std::string &category_name_, CategoriesElem* parent_category_) :
        category_name(category_name_), rating(0), parent_category(parent_category_)
    {
    }
    CategoriesElem(const std::string &category_name_, CategoriesElem* parent_category_, unsigned int rating_) :
        category_name(category_name_), rating(rating_), parent_category(parent_category_)
    {
    }

    std::string category_name;
    unsigned int rating;
    CategoriesElem* parent_category;
    std::list<CategoriesElem*> sub_categories;
    std::list<ExpenseElem> expenses;
};


// TODO: instead of CategoriesElem use Categories with meta info like version
class CategoriesToExtDbConverter {
public:
    virtual void CategoriesToExtDb(CategoriesElem *categories) = 0;
    virtual CategoriesElem * ExtDbToCategories() = 0;
};

// TODO: not core functionality, move to other src files
class CategoriesToJsonConverter {
public:
    // TODO: move semantics
    std::string CategoriesToJsonStr(CategoriesElem *categories);
    CategoriesElem * JsonStrToCategories(const std::string &json_str_categories);
};

class CategoriesToJsonFileConverter final : public CategoriesToExtDbConverter {
public:
    CategoriesToJsonFileConverter(const std::string &json_db_filename);
    void CategoriesToExtDb(CategoriesElem *categories) override;
    CategoriesElem * ExtDbToCategories() override;
private:
    CategoriesToJsonConverter categories_to_json_converter; // TODO: DepInj
    std::string json_db_filename;
};

class CostsNbCore
{
public:
    CostsNbCore(const std::string &db_file_name, const std::string &cfg_file_name);
    ~CostsNbCore();

    std::tuple<std::list<std::string>, std::string> GetCurrentCategories();
    bool CategorySelected(const std::string &selected_category);
    bool RemoveCategory(const std::string &selected_category);
    bool RenameCategory(const std::string &old_name, const std::string &new_name);
    void CategoryBack();
    bool CategoryAdd(const std::string &new_category);
    bool CategoryAddSub(const std::string &parent_category, const std::string &new_category);
    void Buy(const std::string &selected_category, const unsigned int &cost, const std::string &info);
    std::list<std::string> GetExpenses(const std::string &selected_category);
    std::list<std::string> GetAllExpenses(const std::string &selected_category);

private:
    CategoriesElem *categories;
    std::string db_file_name;
    std::string cfg_file_name;

    // TODO: refactor working with json lib
    CategoriesElem* ReadCategoriesFromDb(const std::string &dbFile);
    void ConvertJsonToCategories(CategoriesElem* parentCategory, const unsigned int &index, const Json::Value &jsonCategories);
    void ConvertCategoriesToJson(Json::Value &rootJson, const CategoriesElem* categories);
    void WriteCategoriesToDb(const CategoriesElem* rootCategory, const std::string &db_file_name);
    // TODO: refactor working with server
    void SyncDbWithServer();
};

#endif // COSTS_NB_CORE_H
