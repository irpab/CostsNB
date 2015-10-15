#include <fstream>
#include <string>
#include <iostream>
#include <memory>

//#include <QDebug>

#include "costs_nb_core.h"
#include "categories_to_json_converter.h"
#include "categories_to_backend.h"

bool operator==(const ExpenseElem::Datetime& e1, const ExpenseElem::Datetime& e2) {
  return !( (e1 > e2) || (e2 > e1) );
}

bool operator!=(const ExpenseElem::Datetime& e1, const ExpenseElem::Datetime& e2) {
  return !(e1 == e2);
}

bool operator==(const ExpenseElem& e1, const ExpenseElem& e2) {
  if (e1.datetime != e2.datetime)
    return false;

  if (e1.cost != e2.cost)
    return false;

  if (0 != std::string(e1.info).compare(e2.info))
    return false;

  return true;
}

bool operator!=(const ExpenseElem& e1, const ExpenseElem& e2) {
  return !(e1 == e2);
}

bool operator!=(const CategoriesElem& e1, const CategoriesElem& e2);
bool operator==(const CategoriesElem& e1, const CategoriesElem& e2) {
  if (0 != std::string(e1.category_name).compare(e2.category_name))
    return false;

  if (e1.rating != e2.rating)
    return false;

  if (e1.sub_categories.size() != e2.sub_categories.size())
    return false;
  {
    auto sub_category1 = e1.sub_categories.begin();
    auto sub_category2 = e2.sub_categories.begin();
    for (;sub_category1 != e1.sub_categories.end(), sub_category2 != e2.sub_categories.end();
        ++sub_category1, ++sub_category2) {
      if (**sub_category1 != **sub_category2)
        return false;
    }
  }

  if (e1.expenses.size() != e2.expenses.size())
    return false;
  {
    auto expens1 = e1.expenses.begin();
    auto expens2 = e2.expenses.begin();
    for (;expens1 != e1.expenses.end(), expens2 != e2.expenses.end();
        ++expens1, ++expens2) {
      if (*expens1 != *expens2)
        return false;
    }
  }

  return true;
}

bool operator!=(const CategoriesElem& e1, const CategoriesElem& e2) {
  return !(e1 == e2);
}

CategoriesElem * GetRootCategory(CategoriesElem * const category)
{
    CategoriesElem* root_category = category;
    while (root_category->parent_category != nullptr)
        root_category = root_category->parent_category;
    return root_category;
}

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

std::string RemoveDisplaySubCategoriesPrefix(const std::string& category_name)
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

void Buy(CategoriesElem* categories, const std::string &selected_category, const unsigned int cost, const std::string &info, struct tm * now)
{
    CategoriesElem* category = nullptr;
    if (selected_category.empty())
        category = categories;
    else
        category = GetSubCategoryByName(categories, selected_category);

    if (category == nullptr)
        return;

    category->expenses.push_back(ExpenseElem(now, cost, info));

    while (category != nullptr) {
        category->rating++;
        category->sub_categories.sort(CompareCategoriesByRating);
        category = category->parent_category;
    }
}

void Buy(CategoriesElem* categories, const std::string &selected_category, const unsigned int cost, const std::string &info)
{
    auto now = utils::Now();
    Buy(categories, selected_category, cost, info, now);
}

/////////////////////////////
// API
/////////////////////////////

CostsNbCore::CostsNbCore(CategoriesToExtDbConverter *categories_to_ext_db_converter_, CategoriesToBackend *categories_to_backend_)
  : categories_to_ext_db_converter(categories_to_ext_db_converter_), categories_to_backend(categories_to_backend_)
{
    categories = categories_to_ext_db_converter->ExtDbToCategories();
}

CostsNbCore::~CostsNbCore()
{
    categories_to_ext_db_converter->CategoriesToExtDb(GetRootCategory(categories));
    categories_to_backend->SyncToBackend(GetRootCategory(categories));
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
