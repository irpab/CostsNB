#include <fstream>
#include <string>
#include <memory>
#include <algorithm>

//#include <QDebug>

#include "costs_nb_core.h"
#include "categories_to_json_converter.h"
#include "categories_to_backend.h"

bool operator==(const Datetime& e1, const Datetime& e2) {
  return !( (e1 > e2) || (e2 > e1) );
}

bool operator!=(const Datetime& e1, const Datetime& e2) {
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

CategoriesElem* GetRootCategory(CategoriesElem* const category)
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
  auto found_category = std::find_if(
    category->sub_categories.begin(),
    category->sub_categories.end(),
    [&category_name](const CategoriesElem* const sub_category) {
      return !sub_category->category_name.compare(category_name);
    });
  if (found_category != category->sub_categories.end())
      return *found_category;
  return nullptr;
}

bool SubCategoriesContainCategory(const CategoriesElem* category, const std::string &category_name)
{
  auto found_category = GetSubCategoryByName(category, category_name);
  return found_category != nullptr;
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
  // TODO: use algo whet UT will be ready
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
    expenses.resize(category->expenses.size());
    std::transform(
      category->expenses.begin(),
      category->expenses.end(),
      expenses.begin(),
      [](const ExpenseElem& expense){
        return expense.ToStr();
      });
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
    expenses_str.resize(expenses.size());
    std::transform(
      expenses.begin(),
      expenses.end(),
      expenses_str.begin(),
      [](const ExpenseElem& expense){
        return expense.ToStr();
      });
  }
  return expenses_str;
}

void Buy(CategoriesElem* categories, const std::string &selected_category, const unsigned int cost, const std::string &info, struct tm* now)
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

bool RemoveCategory(CategoriesElem* categories, const std::string &removing_category_name_)
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

  return true;
}

bool MoveCategory(CategoriesElem* current_parent, CategoriesElem* new_parent, const std::string &moving_category_name_)
{
  if (current_parent == new_parent)
    return false;

  std::string moving_category_name = RemoveDisplaySubCategoriesPrefix(moving_category_name_);

  CategoriesElem* moving_category = GetSubCategoryByName(current_parent, moving_category_name);
  if (moving_category == nullptr)
    return false;

  if (new_parent == moving_category)
    return false;

  auto parent_subcat_same_name = GetSubCategoryByName(new_parent, moving_category_name);
  if (parent_subcat_same_name != nullptr)
    return false;

  current_parent->sub_categories.remove(moving_category);
  new_parent->sub_categories.push_back(moving_category);
  moving_category->parent_category = new_parent;
  new_parent->sub_categories.sort(CompareCategoriesByRating);

  return true;
}

bool MoveCategoryBack(CategoriesElem* current_parent, const std::string &moving_category_name)
{
  if (current_parent->IsRootCategory())
    return false;
  auto new_parent = current_parent->parent_category;
  return MoveCategory(current_parent, new_parent, moving_category_name);
}

bool MoveCategoryTo(CategoriesElem* current_parent, const std::string &moving_category_name, const std::string &to_category_name_)
{
  std::string to_category_name = RemoveDisplaySubCategoriesPrefix(to_category_name_);
  auto new_parent = GetSubCategoryByName(current_parent, to_category_name);
  if (new_parent == nullptr)
    return false;
  return MoveCategory(current_parent, new_parent, moving_category_name);
}

/////////////////////////////
// Internal classes
/////////////////////////////

Datetime::Datetime(unsigned short y_, unsigned short mn_, unsigned short d_,
                   unsigned short h_, unsigned short m_, unsigned short s_) :
  y(y_), mn(mn_), d(d_), h(h_), m(m_), s(s_)
{}

Datetime::Datetime(const struct tm* t)
{
  y  = t->tm_year + 1900;
  mn = t->tm_mon + 1;
  d  = t->tm_mday;
  h  = t->tm_hour;
  m  = t->tm_min;
  s  = t->tm_sec;
}

bool Datetime::operator>(const Datetime &r) const {
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

std::string Datetime::ToStr() const
{
  std::stringstream ss;
  ss << y << '-' << utils::MonthNumToStr(mn) << '-'
    << std::setw(2) << std::setfill('0') << d << " "
    << std::setw(2) << std::setfill('0') << h << ":"
    << std::setw(2) << std::setfill('0') << m;
  return ss.str();
}

CategoriesElem::CategoriesElem() :
    category_name("")
  , rating(0)
  , parent_category(nullptr)
{ }

CategoriesElem::CategoriesElem(const std::string &category_name_, CategoriesElem* parent_category_) :
    category_name(category_name_)
  , rating(0)
  , parent_category(parent_category_)
{ }

CategoriesElem::CategoriesElem(const std::string &category_name_, CategoriesElem* parent_category_, unsigned int rating_) :
    category_name(category_name_)
  , rating(rating_)
  , parent_category(parent_category_)
{ }

CategoriesElem::CategoriesElem(const CategoriesElem& copy) :
    category_name(copy.category_name)
  , rating(copy.rating)
  , parent_category(nullptr)
  , expenses(copy.expenses)
{
  for (auto copy_sub_cat: copy.sub_categories) {
    CategoriesElem* new_sub_cat = new CategoriesElem(*copy_sub_cat);
    new_sub_cat->parent_category = this;
    sub_categories.push_back(new_sub_cat);
  }
}

void swap(CategoriesElem& first, CategoriesElem& second)
{
  std::swap(first.category_name, second.category_name);
  std::swap(first.rating, second.rating);
  std::swap(first.parent_category, second.parent_category);
  first.expenses.swap(second.expenses);
  first.sub_categories.swap(second.sub_categories);
}

CategoriesElem& CategoriesElem::operator=(CategoriesElem copy)
{
  swap(*this, copy);
  return *this;
}

CategoriesElem::~CategoriesElem()
{
  while(!sub_categories.empty())
    delete sub_categories.front(), sub_categories.pop_front();
}

bool CategoriesElem::IsRootCategory()
{
  return parent_category == nullptr;
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
  delete categories;
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
  bool RemoveRes = ::RemoveCategory(categories, removing_category_name_);
  if (!RemoveRes)
    return false;

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

bool CostsNbCore::MoveCategoryBack(const std::string &moving_category_name)
{
  return ::MoveCategoryBack(categories, moving_category_name);
}

bool CostsNbCore::MoveCategoryTo(const std::string &moving_category_name, const std::string &to_category_name)
{
  return ::MoveCategoryTo(categories, moving_category_name, to_category_name);
}
