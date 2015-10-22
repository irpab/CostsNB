#define CATCH_CONFIG_MAIN
#include <vector>
#include <cstdio>
#include <fstream>

#include "catch.hpp"

#include "costs_nb_core.h"
#include "categories_to_json_converter.h"
#include "base64.h"
#include "utils.h"
#include "miniz_wrp.h"
#include "ini_config.h"

#define CREATE_LEAF_CATEGORY_R(VAR, ROOT, NAME, RATING) CategoriesElem* VAR = new CategoriesElem(NAME, ROOT, RATING)
#define CREATE_ROOT_CATEGORY_R(VAR, NAME, RATING) CREATE_LEAF_CATEGORY_R(VAR, nullptr, NAME, RATING)
#define CREATE_LEAF_CATEGORY(VAR, ROOT, NAME) CategoriesElem* VAR = new CategoriesElem(NAME, ROOT)
#define CREATE_ROOT_CATEGORY(VAR, NAME) CREATE_LEAF_CATEGORY(VAR, nullptr, NAME)
#define LINK_ROOT_LEAF(ROOT, VAR) ROOT->sub_categories.push_back(VAR)
#define ADD_SUB_CATEGORY(VAR, NAME) VAR->sub_categories.push_back(new CategoriesElem(NAME, VAR))
#define COMPARE_STRINGS(STR1, STR2) (0 == std::string(STR1).compare(STR2))
#define CREATE_EXPENSE(VAR, COST) CREATE_EXPENSE_D(VAR, Datetime(1, 1, 1, 1, 1, 1), COST)
#define CREATE_EXPENSE_D(VAR, DATE, COST) ExpenseElem VAR = ExpenseElem(ExpenseElem::DATE, COST)
#define LINK_EXPENSE(CAT, EXP) CAT->expenses.push_back(EXP);

extern CategoriesElem * GetRootCategory(CategoriesElem * const category);
TEST_CASE( "GetRootCategory", "[internal]" ) {
  CREATE_ROOT_CATEGORY(category, "Cat1");
  CREATE_LEAF_CATEGORY(cat11, category, "Cat11");
  CREATE_LEAF_CATEGORY(cat12, category, "Cat12");
  CREATE_LEAF_CATEGORY(cat111, cat11, "Cat111");
  CREATE_LEAF_CATEGORY(cat112, cat11, "Cat112");
  CREATE_LEAF_CATEGORY(cat1111, cat111, "Cat1111");
  LINK_ROOT_LEAF(category, cat11);
  LINK_ROOT_LEAF(category, cat12);
  LINK_ROOT_LEAF(cat11, cat111);
  LINK_ROOT_LEAF(cat11, cat112);
  LINK_ROOT_LEAF(cat111, cat1111);

  CHECK(category == GetRootCategory(category));
  CHECK(category == GetRootCategory(cat11));
  CHECK(category == GetRootCategory(cat12));
  CHECK(category == GetRootCategory(cat111));
  CHECK(category == GetRootCategory(cat112));
  CHECK(category == GetRootCategory(cat1111));
}

extern bool ValidateCategoryName(const std::string &newCategory);
TEST_CASE( "ValidateCategoryName", "[internal]" ) {
  CHECK(!ValidateCategoryName("") );
  CHECK(!ValidateCategoryName("> ") );
  CHECK(!ValidateCategoryName("> asfsfgds") );
  CHECK( ValidateCategoryName(">") );
  CHECK( ValidateCategoryName(" > asfsfgds") );
  CHECK( ValidateCategoryName("asfs > fgds") );
  CHECK( ValidateCategoryName("a") );
  CHECK( ValidateCategoryName("abc def") );
}

extern bool CompareExpensesByDate(const ExpenseElem &e1, const ExpenseElem &e2);
TEST_CASE( "CompareExpensesByDate", "[internal]" ) {
  CREATE_EXPENSE_D(e1_1, Datetime(2015,  1, 11,  1, 21, 0), 7);
  CREATE_EXPENSE_D(e2_1, Datetime(2015,  1, 11,  1, 21, 0), 7);
  CHECK(!CompareExpensesByDate(e1_1, e2_1));
  
  CREATE_EXPENSE_D(e1_2, Datetime(2015,  1, 11,  8, 21, 0), 7);
  CREATE_EXPENSE_D(e2_2, Datetime(2015,  1, 12,  1, 21, 0), 7);
  CHECK(!CompareExpensesByDate(e1_2, e2_2));
  
  CREATE_EXPENSE_D(e1_3, Datetime(2016,  1, 11,  1, 21, 0), 7);
  CREATE_EXPENSE_D(e2_3, Datetime(2015, 20, 11,  4, 42, 0), 12);
  CHECK( CompareExpensesByDate(e1_3, e2_3));
}

extern bool CompareCategoriesByRating(const CategoriesElem* e1, const CategoriesElem* e2);
TEST_CASE( "CompareCategoriesByRating", "[internal]" ) {
  CREATE_ROOT_CATEGORY_R(c1_1, "c1_1", 12);
  CREATE_ROOT_CATEGORY_R(c2_1, "c2_1", 13);
  CHECK(!CompareCategoriesByRating(c1_1, c2_1) );

  CREATE_ROOT_CATEGORY_R(c1_2, "c1_2", 17);
  CREATE_ROOT_CATEGORY_R(c2_2, "c2_2", 13);
  CHECK( CompareCategoriesByRating(c1_2, c2_2) );
  
  CREATE_ROOT_CATEGORY_R(c1_3, "c1_3", 2);
  CREATE_ROOT_CATEGORY_R(c2_3, "c2_3", 2);
  CHECK(!CompareCategoriesByRating(c1_3, c2_3) );
}

extern bool SubCategoriesContainCategory(const CategoriesElem* categories, const std::string &categoryName);
TEST_CASE( "SubCategoriesContainCategory", "[internal]" ) {
  CREATE_ROOT_CATEGORY(category, "Cat1");
  ADD_SUB_CATEGORY(category, "Cat11");
  ADD_SUB_CATEGORY(category, "Cat12");
  ADD_SUB_CATEGORY(category, "Cat13");

  CHECK(!SubCategoriesContainCategory(category, "Cat1"));
  CHECK(!SubCategoriesContainCategory(category, "Cat2"));
  CHECK(!SubCategoriesContainCategory(category, ""));
  CHECK( SubCategoriesContainCategory(category, "Cat11"));
  CHECK( SubCategoriesContainCategory(category, "Cat12"));
  CHECK( SubCategoriesContainCategory(category, "Cat13"));
  CHECK(!SubCategoriesContainCategory(category, "Cat14"));
  CHECK(!SubCategoriesContainCategory(category, "Cat111"));
  CHECK(!SubCategoriesContainCategory(category, "cat11"));
}

extern std::string AddDisplaySubCategoriesPrefix(const CategoriesElem* category);
TEST_CASE( "AddDisplaySubCategoriesPrefix", "[internal]" ) {
  CREATE_ROOT_CATEGORY(category_w_subcat, "Cat1");
  ADD_SUB_CATEGORY(category_w_subcat, "Cat11");
  CHECK(COMPARE_STRINGS("> Cat1", AddDisplaySubCategoriesPrefix(category_w_subcat)));

  CREATE_ROOT_CATEGORY(category_wo_subcat, "Cat1");
  CHECK(COMPARE_STRINGS("Cat1", AddDisplaySubCategoriesPrefix(category_wo_subcat)));
}

extern std::string RemoveDisplaySubCategoriesPrefix(const std::string& categoryName);
TEST_CASE( "RemoveDisplaySubCategoriesPrefix", "[internal]" ) {
  CHECK(COMPARE_STRINGS(""      , RemoveDisplaySubCategoriesPrefix("")));
  CHECK(COMPARE_STRINGS(">"     , RemoveDisplaySubCategoriesPrefix(">")));
  CHECK(COMPARE_STRINGS("> "    , RemoveDisplaySubCategoriesPrefix("> ")));
  CHECK(COMPARE_STRINGS("C"     , RemoveDisplaySubCategoriesPrefix("> C")));
  CHECK(COMPARE_STRINGS("Cat1"  , RemoveDisplaySubCategoriesPrefix("Cat1")));
  CHECK(COMPARE_STRINGS("Cat1"  , RemoveDisplaySubCategoriesPrefix("> Cat1")));
  CHECK(COMPARE_STRINGS("Cat1> ", RemoveDisplaySubCategoriesPrefix("Cat1> ")));
  CHECK(COMPARE_STRINGS("> Cat1", RemoveDisplaySubCategoriesPrefix("> > Cat1")));
}

extern CategoriesElem* GetSubCategoryByName(const CategoriesElem* category, const std::string &categoryName);
TEST_CASE( "GetSubCategoryByName", "[internal]" ) {
  CREATE_ROOT_CATEGORY(category, "Cat1");
  CREATE_LEAF_CATEGORY(subCat1, category, "Cat11");
  CREATE_LEAF_CATEGORY(subCat2, category, "Cat12");
  CREATE_LEAF_CATEGORY(subCat3, category, "Cat13");
  LINK_ROOT_LEAF(category, subCat1);
  LINK_ROOT_LEAF(category, subCat2);
  LINK_ROOT_LEAF(category, subCat3);

  CHECK(subCat1 == GetSubCategoryByName(category, "Cat11"));
  CHECK(subCat2 == GetSubCategoryByName(category, "Cat12"));
  CHECK(subCat3 == GetSubCategoryByName(category, "Cat13"));
  CHECK(nullptr == GetSubCategoryByName(category, "Cat14"));
}

void UtVerifySubCategories(const std::vector<std::string> sub_categories_str, const CategoriesElem* const category) {
  auto sub_category = category->sub_categories.begin();
  for (auto sub_category_str = sub_categories_str.begin(); sub_category_str != sub_categories_str.end(); ++sub_category_str) {
    REQUIRE(COMPARE_STRINGS(*sub_category_str, (*sub_category)->category_name));
    REQUIRE((*sub_category)->parent_category == category);
    ++sub_category;
  }
  REQUIRE(sub_category == category->sub_categories.end());
}

extern bool AddSubCategory(CategoriesElem* category, const std::string &newCategoryName);
TEST_CASE( "AddSubCategory", "[internal]" ) {
  CREATE_ROOT_CATEGORY(category, "Cat1");
  ADD_SUB_CATEGORY(category, "Cat11");
  ADD_SUB_CATEGORY(category, "Cat12");
  UtVerifySubCategories({"Cat11", "Cat12"}, category);

  REQUIRE( AddSubCategory(category, "Cat13"));
  UtVerifySubCategories({"Cat11", "Cat12", "Cat13"}, category);

  UtVerifySubCategories({"Cat11", "Cat12", "Cat13"}, category);
  REQUIRE(!AddSubCategory(category, "Cat11"));

  UtVerifySubCategories({"Cat11", "Cat12", "Cat13"}, category);
  REQUIRE(!AddSubCategory(category, "Cat13"));
}

TEST_CASE( "MonthNumToStr", "[utils]" ) {
  CHECK(COMPARE_STRINGS("Jan", utils::MonthNumToStr( 1)));
  CHECK(COMPARE_STRINGS("Feb", utils::MonthNumToStr( 2)));
  CHECK(COMPARE_STRINGS("Mar", utils::MonthNumToStr( 3)));
  CHECK(COMPARE_STRINGS("Apr", utils::MonthNumToStr( 4)));
  CHECK(COMPARE_STRINGS("May", utils::MonthNumToStr( 5)));
  CHECK(COMPARE_STRINGS("Jun", utils::MonthNumToStr( 6)));
  CHECK(COMPARE_STRINGS("Jul", utils::MonthNumToStr( 7)));
  CHECK(COMPARE_STRINGS("Aug", utils::MonthNumToStr( 8)));
  CHECK(COMPARE_STRINGS("Sep", utils::MonthNumToStr( 9)));
  CHECK(COMPARE_STRINGS("Oct", utils::MonthNumToStr(10)));
  CHECK(COMPARE_STRINGS("Nov", utils::MonthNumToStr(11)));
  CHECK(COMPARE_STRINGS("Dec", utils::MonthNumToStr(12)));
  CHECK(COMPARE_STRINGS("---", utils::MonthNumToStr(13)));
  CHECK(COMPARE_STRINGS("---", utils::MonthNumToStr( 0)));
}

TEST_CASE( "MonthStrToNum", "[utils]" ) {
  CHECK(utils::MonthStrToNum("xxxxx") ==  0);
  CHECK(utils::MonthStrToNum("xxx") ==  0);
  CHECK(utils::MonthStrToNum("")    ==  0);
  CHECK(utils::MonthStrToNum("Jan") ==  1);
  CHECK(utils::MonthStrToNum("Feb") ==  2);
  CHECK(utils::MonthStrToNum("Mar") ==  3);
  CHECK(utils::MonthStrToNum("Apr") ==  4);
  CHECK(utils::MonthStrToNum("May") ==  5);
  CHECK(utils::MonthStrToNum("Jun") ==  6);
  CHECK(utils::MonthStrToNum("Jul") ==  7);
  CHECK(utils::MonthStrToNum("Aug") ==  8);
  CHECK(utils::MonthStrToNum("Sep") ==  9);
  CHECK(utils::MonthStrToNum("Oct") == 10);
  CHECK(utils::MonthStrToNum("Nov") == 11);
  CHECK(utils::MonthStrToNum("Dec") == 12);
}

extern bool RenameCategory(CategoriesElem* categories, const std::string &oldName0, const std::string &newName);
TEST_CASE( "RenameCategory", "[internal]" ) {
  CREATE_ROOT_CATEGORY(category, "Cat1");
  ADD_SUB_CATEGORY(category, "Cat11");
  ADD_SUB_CATEGORY(category, "Cat12");
  ADD_SUB_CATEGORY(category, "Cat13");
  UtVerifySubCategories({"Cat11", "Cat12", "Cat13"}, category);

  REQUIRE(!RenameCategory(category, "Cat11", "Cat12"));
  REQUIRE(!RenameCategory(category, "Cat12", "Cat12"));
  REQUIRE(!RenameCategory(category, "Cat11", ""));
  UtVerifySubCategories({"Cat11", "Cat12", "Cat13"}, category);

  REQUIRE( RenameCategory(category, "Cat11", "Tac111"));
  REQUIRE( RenameCategory(category, "Cat13", "Cat14"));
  REQUIRE( RenameCategory(category, "Tac111", "Tac0"));
  UtVerifySubCategories({"Tac0", "Cat12", "Cat14"}, category);
}

void UtVerifyExpenses(const std::vector<std::string> expenses0, const std::list<std::string> &expenses1) {
  REQUIRE(expenses0.size() == expenses1.size());
  auto expense_i1 = expenses1.begin();
  for (auto expense_i0 = expenses0.begin(); expense_i0 != expenses0.end(); ++expense_i0) {
    REQUIRE(COMPARE_STRINGS(*expense_i0, *expense_i1));
    ++expense_i1;
  }
  REQUIRE(expense_i1 == expenses1.end());
}

extern std::list<std::string> GetExpenses(CategoriesElem* categories, const std::string &selectedCategory0);
TEST_CASE( "GetExpenses", "[internal]" ) {
  CREATE_ROOT_CATEGORY(category, "Cat1");
  CREATE_LEAF_CATEGORY(cat11, category, "Cat11");
  CREATE_LEAF_CATEGORY(cat12, category, "Cat12");
  LINK_ROOT_LEAF(category, cat11);
  LINK_ROOT_LEAF(category, cat12);

  CREATE_EXPENSE(e11_1, 1);
  CREATE_EXPENSE(e11_2, 132);
  CREATE_EXPENSE(e11_3, 56);
  CREATE_EXPENSE(e12_1, 463);
  CREATE_EXPENSE(e12_2, 65);
  LINK_EXPENSE(cat11, e11_1);
  LINK_EXPENSE(cat11, e11_2);
  LINK_EXPENSE(cat11, e11_3);
  LINK_EXPENSE(cat12, e12_1);
  LINK_EXPENSE(cat12, e12_2);

  UtVerifyExpenses({e11_1.ToStr(), e11_2.ToStr(), e11_3.ToStr()}, GetExpenses(category, "Cat11"));
  UtVerifyExpenses({e12_1.ToStr(), e12_2.ToStr()}, GetExpenses(category, "Cat12"));
  REQUIRE(GetExpenses(category, "Cat10").empty());
}

TEST_CASE( "ExpenseElem.ToStr", "[internal]" ) {
  CREATE_EXPENSE_D(e11_1, Datetime(2012, 12,  1,  5, 38,  0),   1);
  CREATE_EXPENSE_D(e11_2, Datetime(1954,  1, 23, 23, 24, 59), 132);
  CREATE_EXPENSE_D(e11_3, Datetime(2174,  7, 30, 11,  1, 54),  56);

  CHECK(COMPARE_STRINGS("2012-Dec-01 05:38   1   "  , e11_1.ToStr()));
  CHECK(COMPARE_STRINGS("1954-Jan-23 23:24   132   ", e11_2.ToStr()));
  CHECK(COMPARE_STRINGS("2174-Jul-30 11:01   56   " , e11_3.ToStr()));
}

TEST_CASE( "Datetime.operator>", "[internal]" ) {
  CHECK(!(ExpenseElem::Datetime(1, 1, 1, 1, 1, 1) > ExpenseElem::Datetime(1, 1, 1, 1, 1, 1)) );

  CHECK(!(ExpenseElem::Datetime(1, 1, 1, 1, 1, 1) > ExpenseElem::Datetime(2, 1, 1, 1, 1, 1)) );
  CHECK(!(ExpenseElem::Datetime(1, 1, 1, 1, 1, 1) > ExpenseElem::Datetime(1, 2, 1, 1, 1, 1)) );
  CHECK(!(ExpenseElem::Datetime(1, 1, 1, 1, 1, 1) > ExpenseElem::Datetime(1, 1, 2, 1, 1, 1)) );
  CHECK(!(ExpenseElem::Datetime(1, 1, 1, 1, 1, 1) > ExpenseElem::Datetime(1, 1, 1, 2, 1, 1)) );
  CHECK(!(ExpenseElem::Datetime(1, 1, 1, 1, 1, 1) > ExpenseElem::Datetime(1, 1, 1, 1, 2, 1)) );
  CHECK(!(ExpenseElem::Datetime(1, 1, 1, 1, 1, 1) > ExpenseElem::Datetime(1, 1, 1, 1, 1, 2)) );

  CHECK( (ExpenseElem::Datetime(2, 1, 1, 1, 1, 1) > ExpenseElem::Datetime(1, 1, 1, 1, 1, 1)) );
  CHECK( (ExpenseElem::Datetime(1, 2, 1, 1, 1, 1) > ExpenseElem::Datetime(1, 1, 1, 1, 1, 1)) );
  CHECK( (ExpenseElem::Datetime(1, 1, 2, 1, 1, 1) > ExpenseElem::Datetime(1, 1, 1, 1, 1, 1)) );
  CHECK( (ExpenseElem::Datetime(1, 1, 1, 2, 1, 1) > ExpenseElem::Datetime(1, 1, 1, 1, 1, 1)) );
  CHECK( (ExpenseElem::Datetime(1, 1, 1, 1, 2, 1) > ExpenseElem::Datetime(1, 1, 1, 1, 1, 1)) );
  CHECK( (ExpenseElem::Datetime(1, 1, 1, 1, 1, 2) > ExpenseElem::Datetime(1, 1, 1, 1, 1, 1)) );
}

extern std::list<std::string> GetAllExpenses(CategoriesElem* categories, const std::string &selectedCategory0);
TEST_CASE( "GetAllExpenses", "[internal]" ) {
  CREATE_ROOT_CATEGORY(category, "Cat1");
  CREATE_LEAF_CATEGORY(cat11, category, "Cat11");
  CREATE_LEAF_CATEGORY(cat12, category, "Cat12");
  CREATE_LEAF_CATEGORY(cat111, cat11, "Cat111");
  CREATE_LEAF_CATEGORY(cat112, cat11, "Cat112");
  CREATE_LEAF_CATEGORY(cat1111, cat111, "Cat1111");
  LINK_ROOT_LEAF(category, cat11);
  LINK_ROOT_LEAF(category, cat12);
  LINK_ROOT_LEAF(cat11, cat111);
  LINK_ROOT_LEAF(cat11, cat112);
  LINK_ROOT_LEAF(cat111, cat1111);

  CREATE_EXPENSE_D(e1_1   , Datetime(2015,  1, 11,  1, 21, 0), 1);
  CREATE_EXPENSE_D(e1_2   , Datetime(2015,  7,  5,  6, 19, 0), 2);
  CREATE_EXPENSE_D(e11_1  , Datetime(2014,  2, 21, 13, 34, 0), 3);
  CREATE_EXPENSE_D(e11_2  , Datetime(2015,  1,  4,  3, 35, 0), 4);
  CREATE_EXPENSE_D(e12_1  , Datetime(2015,  4,  1,  9, 13, 0), 5);
  CREATE_EXPENSE_D(e111_1 , Datetime(2017,  4,  6,  7, 56, 0), 6);
  CREATE_EXPENSE_D(e112_1 , Datetime(2015,  1,  1,  1,  7, 0), 7);
  CREATE_EXPENSE_D(e112_2 , Datetime(2017,  9, 27, 21,  1, 0), 8);
  CREATE_EXPENSE_D(e1111_1, Datetime(2015, 12,  1, 23,  2, 0), 9);

  LINK_EXPENSE(category, e1_1);
  LINK_EXPENSE(category, e1_2);
  LINK_EXPENSE(cat11, e11_1);
  LINK_EXPENSE(cat11, e11_2);
  LINK_EXPENSE(cat12, e12_1);
  LINK_EXPENSE(cat111, e111_1);
  LINK_EXPENSE(cat112, e112_1);
  LINK_EXPENSE(cat112, e112_2);
  LINK_EXPENSE(cat1111, e1111_1);

  UtVerifyExpenses({e112_2.ToStr(), e111_1.ToStr(), e1111_1.ToStr(),
    e11_2.ToStr(), e112_1.ToStr(), e11_1.ToStr()},
    GetAllExpenses(category, "Cat11"));
}

extern void Buy(CategoriesElem* categories, const std::string &selectedCategory, const unsigned int cost, const std::string &info, struct tm * now);
TEST_CASE( "Buy.rating", "[internal]" ) {
  CREATE_ROOT_CATEGORY_R(cat1, "Cat1", 2);
  CREATE_LEAF_CATEGORY_R(  cat11,   cat1,   "Cat11", 15);
  CREATE_LEAF_CATEGORY_R( cat111,  cat11,  "Cat111",  4);
  CREATE_LEAF_CATEGORY_R(cat1111, cat111, "Cat1111",  9);

  LINK_ROOT_LEAF(  cat1,   cat11);
  LINK_ROOT_LEAF( cat11,  cat111);
  LINK_ROOT_LEAF(cat111, cat1111);

  Buy(cat111, "Cat1111", 7, "", utils::Now());
  CHECK( 3 ==    cat1->rating);
  CHECK(16 ==   cat11->rating);
  CHECK( 5 ==  cat111->rating);
  CHECK(10 == cat1111->rating);
}

TEST_CASE( "Buy.logic", "[internal]" ) {
  CREATE_ROOT_CATEGORY(cat, "Cat");
  CREATE_LEAF_CATEGORY(cat0,  cat, "Cat0");
  CREATE_LEAF_CATEGORY(cat1, cat0, "Cat1");
  CREATE_LEAF_CATEGORY(cat2, cat0, "Cat2");
  CREATE_LEAF_CATEGORY(cat3, cat0, "Cat3");

  LINK_ROOT_LEAF( cat, cat0);
  LINK_ROOT_LEAF(cat0, cat1);
  LINK_ROOT_LEAF(cat0, cat2);
  LINK_ROOT_LEAF(cat0, cat3);

  UtVerifyExpenses({}, GetExpenses( cat, "Cat0"));
  UtVerifyExpenses({}, GetExpenses(cat0, "Cat1"));
  UtVerifyExpenses({}, GetExpenses(cat0, "Cat2"));
  UtVerifyExpenses({}, GetExpenses(cat0, "Cat3"));

  auto now = utils::Now();
  CREATE_EXPENSE_D(exp1, Datetime(now), 35);
  Buy(cat0, "", 35, "", now);
  UtVerifyExpenses({exp1.ToStr()}, GetExpenses( cat, "Cat0"));
  UtVerifyExpenses({}            , GetExpenses(cat0, "Cat1"));
  UtVerifyExpenses({}            , GetExpenses(cat0, "Cat2"));
  UtVerifyExpenses({}            , GetExpenses(cat0, "Cat3"));

  CREATE_EXPENSE_D(exp2, Datetime(now), 76);
  Buy(cat0, "Cat3", 76, "", now);
  UtVerifyExpenses({exp1.ToStr()}, GetExpenses( cat, "Cat0"));
  UtVerifyExpenses({}            , GetExpenses(cat0, "Cat1"));
  UtVerifyExpenses({}            , GetExpenses(cat0, "Cat2"));
  UtVerifyExpenses({exp2.ToStr()}, GetExpenses(cat0, "Cat3"));
}

extern bool RemoveCategory(CategoriesElem* categories, const std::string &removing_category_name_);
TEST_CASE( "RemoveCategory", "[internal]" ) {
  CREATE_ROOT_CATEGORY(category, "Cat1");
  ADD_SUB_CATEGORY(category, "Cat11");
  ADD_SUB_CATEGORY(category, "Cat12");
  ADD_SUB_CATEGORY(category, "Cat13");
  UtVerifySubCategories({"Cat11", "Cat12", "Cat13"}, category);

  REQUIRE(!RemoveCategory(category, "Cat14"));
  REQUIRE(!RemoveCategory(category, "Cat1"));
  REQUIRE(!RemoveCategory(category, ""));
  UtVerifySubCategories({"Cat11", "Cat12", "Cat13"}, category);

  REQUIRE( RemoveCategory(category, "Cat12"));
  UtVerifySubCategories({"Cat11", "Cat13"}, category);
  REQUIRE(!RemoveCategory(category, "Cat12"));

  REQUIRE( RemoveCategory(category, "Cat13"));
  UtVerifySubCategories({"Cat11"}, category);

  REQUIRE( RemoveCategory(category, "Cat11"));
  UtVerifySubCategories({}, category);
}

extern bool MoveCategoryBack(CategoriesElem* categories, const std::string &moving_category_name_);
TEST_CASE( "MoveCategoryBack", "[internal]" ) {
  CREATE_ROOT_CATEGORY(category, "Cat1");
  CREATE_LEAF_CATEGORY(cat11, category, "Cat11");
  CREATE_LEAF_CATEGORY(cat12, category, "Cat112");
  CREATE_LEAF_CATEGORY(cat111, cat11, "Cat111");
  CREATE_LEAF_CATEGORY(cat112, cat11, "Cat112");
  CREATE_LEAF_CATEGORY(cat1111, cat111, "Cat1111");
  LINK_ROOT_LEAF(category, cat11);
  LINK_ROOT_LEAF(category, cat12);
  LINK_ROOT_LEAF(cat11, cat111);
  LINK_ROOT_LEAF(cat11, cat112);
  LINK_ROOT_LEAF(cat111, cat1111);

  UtVerifySubCategories({"Cat11", "Cat112"}, category);
  UtVerifySubCategories({"Cat111", "Cat112"}, cat11);
  UtVerifySubCategories({}, cat12);
  UtVerifySubCategories({"Cat1111"}, cat111);
  UtVerifySubCategories({}, cat112);

  REQUIRE(!MoveCategoryBack(category, "Cat112"));
  REQUIRE(!MoveCategoryBack(cat11, "Cat112"));
  REQUIRE( MoveCategoryBack(cat11, "Cat111"));

  UtVerifySubCategories({"Cat11", "Cat112", "Cat111"}, category);
  UtVerifySubCategories({"Cat112"}, cat11);
  UtVerifySubCategories({}, cat12);
  UtVerifySubCategories({"Cat1111"}, cat111);
  UtVerifySubCategories({}, cat112);
}

extern bool MoveCategoryTo(CategoriesElem* categories, const std::string &moving_category_name, const std::string &to_category_name);
TEST_CASE( "MoveCategoryTo", "[internal]" ) {
  CREATE_ROOT_CATEGORY(category, "Cat1");
  CREATE_LEAF_CATEGORY(cat11, category, "Cat11");
  CREATE_LEAF_CATEGORY(cat111, cat11, "Cat111");
  CREATE_LEAF_CATEGORY(cat112, cat11, "Cat112");
  CREATE_LEAF_CATEGORY(cat113, cat11, "Cat1121");
  CREATE_LEAF_CATEGORY(cat1111, cat111, "Cat1111");
  CREATE_LEAF_CATEGORY(cat1121, cat112, "Cat1121");
  LINK_ROOT_LEAF(category, cat11);
  LINK_ROOT_LEAF(cat11, cat111);
  LINK_ROOT_LEAF(cat11, cat112);
  LINK_ROOT_LEAF(cat11, cat113);
  LINK_ROOT_LEAF(cat111, cat1111);
  LINK_ROOT_LEAF(cat112, cat1121);

  UtVerifySubCategories({"Cat11"}, category);
  UtVerifySubCategories({"Cat111", "Cat112", "Cat1121"}, cat11);
  UtVerifySubCategories({"Cat1111"}, cat111);
  UtVerifySubCategories({"Cat1121"}, cat112);

  REQUIRE(!MoveCategoryTo(cat11, "Cat1121", "Cat112"));
  REQUIRE(!MoveCategoryTo(cat11, "Cat111", "Cat111"));
  REQUIRE( MoveCategoryTo(cat11, "Cat111", "Cat112"));

  UtVerifySubCategories({"Cat11"}, category);
  UtVerifySubCategories({"Cat112", "Cat1121"}, cat11);
  UtVerifySubCategories({"Cat1111"}, cat111);
  UtVerifySubCategories({"Cat1121", "Cat111"}, cat112);
}

// TODO: void GetAllNestedExpenses(std::list<ExpenseElem> &expenses, const CategoriesElem* category);


// Converters UT

TEST_CASE( "ExpenseElem::Datetime.==", "[internal]" ) {
  ExpenseElem::Datetime d1(2015,  1, 11,  1, 21, 0);
  ExpenseElem::Datetime d2(2015,  1, 11,  1, 21, 0);
  ExpenseElem::Datetime d3(2015,  1, 11,  1, 14, 0);
  ExpenseElem::Datetime d4(2015,  2, 11,  1, 21, 0);

  CHECK(d1 == d2);
  CHECK(d2 == d1);
  CHECK(d1 != d3);
  CHECK(d1 != d4);
  CHECK(d3 != d4);
  CHECK(d4 != d3);
}

TEST_CASE( "ExpenseElem.==", "[internal]" ) {
  CREATE_EXPENSE_D(e1, Datetime(2015,  1, 11,  1, 21, 0), 7);
  CREATE_EXPENSE_D(e2, Datetime(2015,  1, 11,  1, 21, 0), 7);
  CREATE_EXPENSE_D(e3, Datetime(2015,  1, 11,  1, 21, 0), 9);
  CREATE_EXPENSE_D(e4, Datetime(2015,  1,  4,  1, 21, 0), 7);

  CHECK(e1 == e2);
  CHECK(e2 == e1);
  CHECK(e1 != e3);
  CHECK(e3 != e1);
  CHECK(e1 != e4);
  CHECK(e3 != e4);
}

TEST_CASE( "CategoriesElem.==", "[internal]" ) {
  CREATE_ROOT_CATEGORY_R(cat1_1, "Cat1", 7);
  CREATE_LEAF_CATEGORY_R(cat1_11, cat1_1, "Cat11", 2);
  CREATE_LEAF_CATEGORY_R(cat1_12, cat1_1, "Cat12", 0);
  CREATE_LEAF_CATEGORY_R(cat1_121, cat1_12, "Cat121", 32);
  LINK_ROOT_LEAF(cat1_1, cat1_11);
  LINK_ROOT_LEAF(cat1_1, cat1_12);
  LINK_ROOT_LEAF(cat1_12, cat1_121);
  CREATE_EXPENSE_D(e1_1_1, Datetime(2015,  1,  2, 12, 28, 0), 1423);
  CREATE_EXPENSE_D(e1_11_1, Datetime(2014,  6, 23, 21, 21, 0), 2);
  CREATE_EXPENSE_D(e1_11_2, Datetime(2012, 12, 16,  7, 54, 0), 54632);
  CREATE_EXPENSE_D(e1_121_1, Datetime(2015,  1,  9,  1, 21, 0), 43);
  CREATE_EXPENSE_D(e1_121_2, Datetime(2013,  2,  3, 14, 12, 0), 243);
  LINK_EXPENSE(cat1_1, e1_1_1);
  LINK_EXPENSE(cat1_11, e1_11_1);
  LINK_EXPENSE(cat1_11, e1_11_2);
  LINK_EXPENSE(cat1_121, e1_121_1);
  LINK_EXPENSE(cat1_121, e1_121_2);

  CREATE_ROOT_CATEGORY_R(cat2_1, "Cat1", 7);
  CREATE_LEAF_CATEGORY_R(cat2_11, cat2_1, "Cat11", 2);
  CREATE_LEAF_CATEGORY_R(cat2_12, cat2_1, "Cat12", 0);
  CREATE_LEAF_CATEGORY_R(cat2_121, cat2_12, "Cat121", 32);
  LINK_ROOT_LEAF(cat2_1, cat2_11);
  LINK_ROOT_LEAF(cat2_1, cat2_12);
  LINK_ROOT_LEAF(cat2_12, cat2_121);
  CREATE_EXPENSE_D(e2_1_1, Datetime(2015,  1,  2, 12, 28, 0), 1423);
  CREATE_EXPENSE_D(e2_11_1, Datetime(2014,  6, 23, 21, 21, 0), 2);
  CREATE_EXPENSE_D(e2_11_2, Datetime(2012, 12, 16,  7, 54, 0), 54632);
  CREATE_EXPENSE_D(e2_121_1, Datetime(2015,  1,  9,  1, 21, 0), 43);
  CREATE_EXPENSE_D(e2_121_2, Datetime(2013,  2,  3, 14, 12, 0), 243);
  LINK_EXPENSE(cat2_1, e2_1_1);
  LINK_EXPENSE(cat2_11, e2_11_1);
  LINK_EXPENSE(cat2_11, e2_11_2);
  LINK_EXPENSE(cat2_121, e2_121_1);
  LINK_EXPENSE(cat2_121, e2_121_2);

  CREATE_ROOT_CATEGORY_R(cat3_1, "Cat1", 7);
  CREATE_LEAF_CATEGORY_R(cat3_11, cat3_1, "Cat11", 2);
  CREATE_LEAF_CATEGORY_R(cat3_12, cat3_1, "Cat12", 0);
  LINK_ROOT_LEAF(cat3_1, cat3_11);
  LINK_ROOT_LEAF(cat3_1, cat3_12);
  CREATE_EXPENSE_D(e3_1_1, Datetime(2015,  1,  2, 12, 28, 0), 1423);
  CREATE_EXPENSE_D(e3_11_1, Datetime(2014,  6, 23, 21, 21, 0), 2);
  CREATE_EXPENSE_D(e3_11_2, Datetime(2012, 12, 16,  7, 54, 0), 54632);
  LINK_EXPENSE(cat3_1, e3_1_1);
  LINK_EXPENSE(cat3_11, e3_11_1);
  LINK_EXPENSE(cat3_11, e3_11_2);

  // name
  CREATE_ROOT_CATEGORY_R(cat4_1, "Cat1x", 7);
  CREATE_LEAF_CATEGORY_R(cat4_11, cat4_1, "Cat11", 2);
  CREATE_LEAF_CATEGORY_R(cat4_12, cat4_1, "Cat12", 0);
  CREATE_LEAF_CATEGORY_R(cat4_121, cat4_12, "Cat121", 32);
  LINK_ROOT_LEAF(cat4_1, cat4_11);
  LINK_ROOT_LEAF(cat4_1, cat4_12);
  LINK_ROOT_LEAF(cat4_12, cat4_121);
  CREATE_EXPENSE_D(e4_1_1, Datetime(2015,  1,  2, 12, 28, 0), 1423);
  CREATE_EXPENSE_D(e4_11_1, Datetime(2014,  6, 23, 21, 21, 0), 2);
  CREATE_EXPENSE_D(e4_11_2, Datetime(2012, 12, 16,  7, 54, 0), 54632);
  CREATE_EXPENSE_D(e4_121_1, Datetime(2015,  1,  9,  1, 21, 0), 43);
  CREATE_EXPENSE_D(e4_121_2, Datetime(2013,  2,  3, 14, 12, 0), 243);
  LINK_EXPENSE(cat4_1, e4_1_1);
  LINK_EXPENSE(cat4_11, e4_11_1);
  LINK_EXPENSE(cat4_11, e4_11_2);
  LINK_EXPENSE(cat4_121, e4_121_1);
  LINK_EXPENSE(cat4_121, e4_121_2);

  // rating
  CREATE_ROOT_CATEGORY_R(cat5_1, "Cat1", 9);
  CREATE_LEAF_CATEGORY_R(cat5_11, cat5_1, "Cat11", 2);
  CREATE_LEAF_CATEGORY_R(cat5_12, cat5_1, "Cat12", 0);
  CREATE_LEAF_CATEGORY_R(cat5_121, cat5_12, "Cat121", 32);
  LINK_ROOT_LEAF(cat5_1, cat5_11);
  LINK_ROOT_LEAF(cat5_1, cat5_12);
  LINK_ROOT_LEAF(cat5_12, cat5_121);
  CREATE_EXPENSE_D(e5_1_1, Datetime(2015,  1,  2, 12, 28, 0), 1423);
  CREATE_EXPENSE_D(e5_11_1, Datetime(2014,  6, 23, 21, 21, 0), 2);
  CREATE_EXPENSE_D(e5_11_2, Datetime(2012, 12, 16,  7, 54, 0), 54632);
  CREATE_EXPENSE_D(e5_121_1, Datetime(2015,  1,  9,  1, 21, 0), 43);
  CREATE_EXPENSE_D(e5_121_2, Datetime(2013,  2,  3, 14, 12, 0), 243);
  LINK_EXPENSE(cat5_1, e5_1_1);
  LINK_EXPENSE(cat5_11, e5_11_1);
  LINK_EXPENSE(cat5_11, e5_11_2);
  LINK_EXPENSE(cat5_121, e5_121_1);
  LINK_EXPENSE(cat5_121, e5_121_2);

  // sub_cat size
  CREATE_ROOT_CATEGORY_R(cat6_1, "Cat1", 7);
  CREATE_LEAF_CATEGORY_R(cat6_12, cat6_1, "Cat12", 0);
  CREATE_LEAF_CATEGORY_R(cat6_121, cat6_12, "Cat121", 32);
  LINK_ROOT_LEAF(cat6_1, cat6_12);
  LINK_ROOT_LEAF(cat6_12, cat6_121);
  CREATE_EXPENSE_D(e6_1_1, Datetime(2015,  1,  2, 12, 28, 0), 1423);
  CREATE_EXPENSE_D(e6_121_1, Datetime(2015,  1,  9,  1, 21, 0), 43);
  CREATE_EXPENSE_D(e6_121_2, Datetime(2013,  2,  3, 14, 12, 0), 243);
  LINK_EXPENSE(cat6_1, e6_1_1);
  LINK_EXPENSE(cat6_121, e6_121_1);
  LINK_EXPENSE(cat6_121, e6_121_2);

  // exp size
  CREATE_ROOT_CATEGORY_R(cat7_1, "Cat1", 7);
  CREATE_LEAF_CATEGORY_R(cat7_11, cat7_1, "Cat11", 2);
  CREATE_LEAF_CATEGORY_R(cat7_12, cat7_1, "Cat12", 0);
  CREATE_LEAF_CATEGORY_R(cat7_121, cat7_12, "Cat121", 32);
  LINK_ROOT_LEAF(cat7_1, cat7_11);
  LINK_ROOT_LEAF(cat7_1, cat7_12);
  LINK_ROOT_LEAF(cat7_12, cat7_121);
  CREATE_EXPENSE_D(e7_1_1, Datetime(2015,  1,  2, 12, 28, 0), 1423);
  CREATE_EXPENSE_D(e7_11_1, Datetime(2014,  6, 23, 21, 21, 0), 2);
  CREATE_EXPENSE_D(e7_11_2, Datetime(2012, 12, 16,  7, 54, 0), 54632);
  CREATE_EXPENSE_D(e7_121_1, Datetime(2015,  1,  9,  1, 21, 0), 43);
  LINK_EXPENSE(cat7_1, e7_1_1);
  LINK_EXPENSE(cat7_11, e7_11_1);
  LINK_EXPENSE(cat7_11, e7_11_2);
  LINK_EXPENSE(cat7_121, e7_121_1);

  // name in internal sub_cat
  CREATE_ROOT_CATEGORY_R(cat8_1, "Cat1", 7);
  CREATE_LEAF_CATEGORY_R(cat8_11, cat8_1, "Cat11", 2);
  CREATE_LEAF_CATEGORY_R(cat8_12, cat8_1, "Cat12x", 0);
  CREATE_LEAF_CATEGORY_R(cat8_121, cat8_12, "Cat121", 32);
  LINK_ROOT_LEAF(cat8_1, cat8_11);
  LINK_ROOT_LEAF(cat8_1, cat8_12);
  LINK_ROOT_LEAF(cat8_12, cat8_121);
  CREATE_EXPENSE_D(e8_1_1, Datetime(2015,  1,  2, 12, 28, 0), 1423);
  CREATE_EXPENSE_D(e8_11_1, Datetime(2014,  6, 23, 21, 21, 0), 2);
  CREATE_EXPENSE_D(e8_11_2, Datetime(2012, 12, 16,  7, 54, 0), 54632);
  CREATE_EXPENSE_D(e8_121_1, Datetime(2015,  1,  9,  1, 21, 0), 43);
  CREATE_EXPENSE_D(e8_121_2, Datetime(2013,  2,  3, 14, 12, 0), 243);
  LINK_EXPENSE(cat8_1, e8_1_1);
  LINK_EXPENSE(cat8_11, e8_11_1);
  LINK_EXPENSE(cat8_11, e8_11_2);
  LINK_EXPENSE(cat8_121, e8_121_1);
  LINK_EXPENSE(cat8_121, e8_121_2);

  // rating in internal sub_cat
  CREATE_ROOT_CATEGORY_R(cat9_1, "Cat1", 7);
  CREATE_LEAF_CATEGORY_R(cat9_11, cat9_1, "Cat11", 2);
  CREATE_LEAF_CATEGORY_R(cat9_12, cat9_1, "Cat12", 0);
  CREATE_LEAF_CATEGORY_R(cat9_121, cat9_12, "Cat121", 4);
  LINK_ROOT_LEAF(cat9_1, cat9_11);
  LINK_ROOT_LEAF(cat9_1, cat9_12);
  LINK_ROOT_LEAF(cat9_12, cat9_121);
  CREATE_EXPENSE_D(e9_1_1, Datetime(2015,  1,  2, 12, 28, 0), 1423);
  CREATE_EXPENSE_D(e9_11_1, Datetime(2014,  6, 23, 21, 21, 0), 2);
  CREATE_EXPENSE_D(e9_11_2, Datetime(2012, 12, 16,  7, 54, 0), 54632);
  CREATE_EXPENSE_D(e9_121_1, Datetime(2015,  1,  9,  1, 21, 0), 43);
  CREATE_EXPENSE_D(e9_121_2, Datetime(2013,  2,  3, 14, 12, 0), 243);
  LINK_EXPENSE(cat9_1, e9_1_1);
  LINK_EXPENSE(cat9_11, e9_11_1);
  LINK_EXPENSE(cat9_11, e9_11_2);
  LINK_EXPENSE(cat9_121, e9_121_1);
  LINK_EXPENSE(cat9_121, e9_121_2);

  // expense
  CREATE_ROOT_CATEGORY_R(cat10_1, "Cat1", 7);
  CREATE_LEAF_CATEGORY_R(cat10_11, cat10_1, "Cat11", 2);
  CREATE_LEAF_CATEGORY_R(cat10_12, cat10_1, "Cat12", 0);
  CREATE_LEAF_CATEGORY_R(cat10_121, cat10_12, "Cat121", 32);
  LINK_ROOT_LEAF(cat10_1, cat10_11);
  LINK_ROOT_LEAF(cat10_1, cat10_12);
  LINK_ROOT_LEAF(cat10_12, cat10_121);
  CREATE_EXPENSE_D(e10_1_1, Datetime(2015,  1,  2, 12, 28, 0), 34);
  CREATE_EXPENSE_D(e10_11_1, Datetime(2014,  6, 23, 21, 21, 0), 2);
  CREATE_EXPENSE_D(e10_11_2, Datetime(2012, 12, 16,  7, 54, 0), 54632);
  CREATE_EXPENSE_D(e10_121_1, Datetime(2015,  1,  9,  1, 21, 0), 43);
  CREATE_EXPENSE_D(e10_121_2, Datetime(2013,  2,  3, 14, 12, 0), 243);
  LINK_EXPENSE(cat10_1, e10_1_1);
  LINK_EXPENSE(cat10_11, e10_11_1);
  LINK_EXPENSE(cat10_11, e10_11_2);
  LINK_EXPENSE(cat10_121, e10_121_1);
  LINK_EXPENSE(cat10_121, e10_121_2);

  // expense in internal sub_cat
  CREATE_ROOT_CATEGORY_R(cat11_1, "Cat1", 7);
  CREATE_LEAF_CATEGORY_R(cat11_11, cat11_1, "Cat11", 2);
  CREATE_LEAF_CATEGORY_R(cat11_12, cat11_1, "Cat12", 0);
  CREATE_LEAF_CATEGORY_R(cat11_121, cat11_12, "Cat121", 32);
  LINK_ROOT_LEAF(cat11_1, cat11_11);
  LINK_ROOT_LEAF(cat11_1, cat11_12);
  LINK_ROOT_LEAF(cat11_12, cat11_121);
  CREATE_EXPENSE_D(e11_1_1, Datetime(2015,  1,  2, 12, 28, 0), 1423);
  CREATE_EXPENSE_D(e11_11_1, Datetime(2014,  6, 23, 21, 21, 0), 2);
  CREATE_EXPENSE_D(e11_11_2, Datetime(2012, 12, 16,  7, 54, 0), 54632);
  CREATE_EXPENSE_D(e11_121_1, Datetime(2015,  1,  4,  1, 21, 0), 43);
  CREATE_EXPENSE_D(e11_121_2, Datetime(2013,  2,  3, 14, 12, 0), 243);
  LINK_EXPENSE(cat11_1, e11_1_1);
  LINK_EXPENSE(cat11_11, e11_11_1);
  LINK_EXPENSE(cat11_11, e11_11_2);
  LINK_EXPENSE(cat11_121, e11_121_1);
  LINK_EXPENSE(cat11_121, e11_121_2);

  CHECK(*cat1_1 == *cat2_1);
  CHECK(*cat1_1 != *cat3_1);
  CHECK(*cat1_1 != *cat4_1);
  CHECK(*cat1_1 != *cat5_1);
  CHECK(*cat1_1 != *cat6_1);
  CHECK(*cat1_1 != *cat7_1);
  CHECK(*cat1_1 != *cat8_1);
  CHECK(*cat1_1 != *cat9_1);
  CHECK(*cat1_1 != *cat10_1);
  CHECK(*cat1_1 != *cat11_1);
}

TEST_CASE( "JsoncppLib.CategoriesToJsonStr", "[ext_db_converter]" ) {
  CREATE_ROOT_CATEGORY(category, "Cat1");
  CREATE_LEAF_CATEGORY(cat11, category, "Cat11");
  CREATE_LEAF_CATEGORY(cat12, category, "Cat12");
  CREATE_LEAF_CATEGORY(cat111, cat11, "Cat111");
  CREATE_LEAF_CATEGORY(cat112, cat11, "Cat112");
  CREATE_LEAF_CATEGORY(cat1111, cat111, "Cat1111");
  LINK_ROOT_LEAF(category, cat11);
  LINK_ROOT_LEAF(category, cat12);
  LINK_ROOT_LEAF(cat11, cat111);
  LINK_ROOT_LEAF(cat11, cat112);
  LINK_ROOT_LEAF(cat111, cat1111);

  CREATE_EXPENSE_D(e1_1   , Datetime(2015,  1, 11,  1, 21, 0), 1);
  CREATE_EXPENSE_D(e1_2   , Datetime(2015,  7,  5,  6, 19, 0), 2);
  CREATE_EXPENSE_D(e11_1  , Datetime(2014,  2, 21, 13, 34, 0), 3);
  CREATE_EXPENSE_D(e11_2  , Datetime(2015,  1,  4,  3, 35, 0), 4);
  CREATE_EXPENSE_D(e12_1  , Datetime(2015,  4,  1,  9, 13, 0), 5);
  CREATE_EXPENSE_D(e111_1 , Datetime(2017,  4,  6,  7, 56, 0), 6);
  CREATE_EXPENSE_D(e112_1 , Datetime(2015,  1,  1,  1,  7, 0), 7);
  CREATE_EXPENSE_D(e112_2 , Datetime(2017,  9, 27, 21,  1, 0), 8);
  CREATE_EXPENSE_D(e1111_1, Datetime(2015, 12,  1, 23,  2, 0), 9);

  LINK_EXPENSE(category, e1_1);
  LINK_EXPENSE(category, e1_2);
  LINK_EXPENSE(cat11, e11_1);
  LINK_EXPENSE(cat11, e11_2);
  LINK_EXPENSE(cat12, e12_1);
  LINK_EXPENSE(cat111, e111_1);
  LINK_EXPENSE(cat112, e112_1);
  LINK_EXPENSE(cat112, e112_2);
  LINK_EXPENSE(cat1111, e1111_1);

  CategoriesToJsonConverter *converter_to_json = new CategoriesToJsonConverterJsoncppLib();
  std::string json_str = converter_to_json->CategoriesToJsonStr(category);

  CategoriesToJsonConverter *converter_from_json = new CategoriesToJsonConverterJsoncppLib();
  CategoriesElem* category_after_convert = converter_from_json->JsonStrToCategories(json_str);

  CategoriesToJsonConverter *converter_to_json2 = new CategoriesToJsonConverterJsoncppLib();
  std::string json_str2 = converter_to_json2->CategoriesToJsonStr(category_after_convert);

  REQUIRE(*category_after_convert == *category);
  REQUIRE(COMPARE_STRINGS(json_str, json_str2));
}

TEST_CASE( "CategoriesToJsonFileConverter", "[ext_db_converter]" ) {
  CREATE_ROOT_CATEGORY(category, "Cat1");
  CREATE_LEAF_CATEGORY(cat11, category, "Cat11");
  CREATE_LEAF_CATEGORY(cat12, category, "Cat12");
  CREATE_LEAF_CATEGORY(cat111, cat11, "Cat111");
  CREATE_LEAF_CATEGORY(cat112, cat11, "Cat112");
  CREATE_LEAF_CATEGORY(cat1111, cat111, "Cat1111");
  LINK_ROOT_LEAF(category, cat11);
  LINK_ROOT_LEAF(category, cat12);
  LINK_ROOT_LEAF(cat11, cat111);
  LINK_ROOT_LEAF(cat11, cat112);
  LINK_ROOT_LEAF(cat111, cat1111);

  CREATE_EXPENSE_D(e1_1   , Datetime(2015,  1, 11,  1, 21, 0), 1);
  CREATE_EXPENSE_D(e1_2   , Datetime(2015,  7,  5,  6, 19, 0), 2);
  CREATE_EXPENSE_D(e11_1  , Datetime(2014,  2, 21, 13, 34, 0), 3);
  CREATE_EXPENSE_D(e11_2  , Datetime(2015,  1,  4,  3, 35, 0), 4);
  CREATE_EXPENSE_D(e12_1  , Datetime(2015,  4,  1,  9, 13, 0), 5);
  CREATE_EXPENSE_D(e111_1 , Datetime(2017,  4,  6,  7, 56, 0), 6);
  CREATE_EXPENSE_D(e112_1 , Datetime(2015,  1,  1,  1,  7, 0), 7);
  CREATE_EXPENSE_D(e112_2 , Datetime(2017,  9, 27, 21,  1, 0), 8);
  CREATE_EXPENSE_D(e1111_1, Datetime(2015, 12,  1, 23,  2, 0), 9);

  LINK_EXPENSE(category, e1_1);
  LINK_EXPENSE(category, e1_2);
  LINK_EXPENSE(cat11, e11_1);
  LINK_EXPENSE(cat11, e11_2);
  LINK_EXPENSE(cat12, e12_1);
  LINK_EXPENSE(cat111, e111_1);
  LINK_EXPENSE(cat112, e112_1);
  LINK_EXPENSE(cat112, e112_2);
  LINK_EXPENSE(cat1111, e1111_1);

  std::string json_db_filename("/tmp/test.txt");

  CategoriesToJsonConverter *converter_to_json = new CategoriesToJsonConverterJsoncppLib();
  CategoriesToJsonFileConverter categories_to_json_file_converter(json_db_filename, converter_to_json);
  categories_to_json_file_converter.CategoriesToExtDb(category);

  CategoriesToJsonConverter *converter_from_json = new CategoriesToJsonConverterJsoncppLib();
  CategoriesToJsonFileConverter categories_from_json_file_converter(json_db_filename, converter_from_json);
  CategoriesElem * category_after_convert = categories_from_json_file_converter.ExtDbToCategories();

  REQUIRE(*category_after_convert == *category);
}

class DummyCategoriesToBackend : public CategoriesToBackend {
public:
  bool SyncToBackend(CategoriesElem *categories) { return true; }
};

TEST_CASE( "CostsNbCore.Read_Write_DB", "[complex]" ) {
  CREATE_ROOT_CATEGORY(category, "Cat1");
  CREATE_LEAF_CATEGORY(cat11, category, "Cat11");
  CREATE_LEAF_CATEGORY(cat12, category, "Cat12");
  CREATE_LEAF_CATEGORY(cat111, cat11, "Cat111");
  CREATE_LEAF_CATEGORY(cat112, cat11, "Cat112");
  CREATE_LEAF_CATEGORY(cat1111, cat111, "Cat1111");
  LINK_ROOT_LEAF(category, cat11);
  LINK_ROOT_LEAF(category, cat12);
  LINK_ROOT_LEAF(cat11, cat111);
  LINK_ROOT_LEAF(cat11, cat112);
  LINK_ROOT_LEAF(cat111, cat1111);

  CREATE_EXPENSE_D(e1_1   , Datetime(2015,  1, 11,  1, 21, 0), 1);
  CREATE_EXPENSE_D(e1_2   , Datetime(2015,  7,  5,  6, 19, 0), 2);
  CREATE_EXPENSE_D(e11_1  , Datetime(2014,  2, 21, 13, 34, 0), 3);
  CREATE_EXPENSE_D(e11_2  , Datetime(2015,  1,  4,  3, 35, 0), 4);
  CREATE_EXPENSE_D(e12_1  , Datetime(2015,  4,  1,  9, 13, 0), 5);
  CREATE_EXPENSE_D(e111_1 , Datetime(2017,  4,  6,  7, 56, 0), 6);
  CREATE_EXPENSE_D(e112_1 , Datetime(2015,  1,  1,  1,  7, 0), 7);
  CREATE_EXPENSE_D(e112_2 , Datetime(2017,  9, 27, 21,  1, 0), 8);
  CREATE_EXPENSE_D(e1111_1, Datetime(2015, 12,  1, 23,  2, 0), 9);

  LINK_EXPENSE(category, e1_1);
  LINK_EXPENSE(category, e1_2);
  LINK_EXPENSE(cat11, e11_1);
  LINK_EXPENSE(cat11, e11_2);
  LINK_EXPENSE(cat12, e12_1);
  LINK_EXPENSE(cat111, e111_1);
  LINK_EXPENSE(cat112, e112_1);
  LINK_EXPENSE(cat112, e112_2);
  LINK_EXPENSE(cat1111, e1111_1);

  std::string json_db_filename("/tmp/test.txt");

  CategoriesToJsonFileConverter * init_converter =
    new CategoriesToJsonFileConverter(json_db_filename,
      new CategoriesToJsonConverterJsoncppLib());
  init_converter->CategoriesToExtDb(category);

  CategoriesToJsonFileConverter * converter =
    new CategoriesToJsonFileConverter(json_db_filename,
      new CategoriesToJsonConverterJsoncppLib());
  CostsNbCore * costs_nb_core = new CostsNbCore(converter, new DummyCategoriesToBackend());
  std::remove(json_db_filename.c_str());
  delete costs_nb_core;

  CategoriesElem * category_after_core_work = init_converter->ExtDbToCategories();

  REQUIRE(*category_after_core_work == *category);
}

TEST_CASE( "base64", "[utils]" ) {
  std::string init_str = "hello { world! \"data\" = 123; }";
  auto base64_str = base64_encode(reinterpret_cast<const unsigned char*>(init_str.c_str()), init_str.size());
  auto decoded_str = base64_decode(base64_str);
  CHECK(COMPARE_STRINGS(init_str, decoded_str));
}

TEST_CASE( "miniz", "[utils]" ) {
  std::string init_str = "hello { world! \"data\" = 123; }";
  auto zip_str = compress_string(init_str);
  auto decoded_str = decompress_string(zip_str);
  CHECK(COMPARE_STRINGS(init_str, decoded_str));
}


std::string DeEscapeJsonString(const std::string& input) {
  std::ostringstream ss;
  for (auto iter = input.cbegin(); iter != input.cend(); iter++) {
    switch (*iter) {
      case '\\': ss << ""; break;
      default: ss << *iter; break;
    }
  }
  return ss.str();
}

// extern std::string EscapeJsonString(const std::string& input);
TEST_CASE( "EscapeJsonString", "[utils]" ) {
  auto json_str = "test test { \"json_data\" = data_test }, { }";
  auto escaped_str = utils::EscapeJsonString(json_str);
  auto zipped_str = compress_string(escaped_str);
  auto base64_str = base64_encode(reinterpret_cast<const unsigned char*>(zipped_str.c_str()), zipped_str.size());

  auto de_base64_str = base64_decode(base64_str);
  auto de_zipped_str = decompress_string(de_base64_str);
  auto de_escaped_str = DeEscapeJsonString(de_zipped_str);
  auto de_json_str = de_escaped_str;

  CHECK(COMPARE_STRINGS(json_str, de_json_str));
}

TEST_CASE( "IniConfig", "[utils]" ) {
  std::string cfg_path = "/tmp/ini_cfg.test";

  std::ofstream cfg(cfg_path);
  cfg << "[section1]" << std::endl;
  cfg << "key1 = value1" << std::endl;
  cfg << "key2 = value2" << std::endl;
  cfg << "[section2]" << std::endl;
  cfg << "key1 = value3" << std::endl;
  cfg.close();

  AbstractConfig* ini_cfg = new IniConfig(cfg_path);
  CHECK(COMPARE_STRINGS("value1", ini_cfg->GetValue("section1", "key1")));
  CHECK(COMPARE_STRINGS("value2", ini_cfg->GetValue("section1", "key2")));
  CHECK(COMPARE_STRINGS(""      , ini_cfg->GetValue("section1", "key3")));
  CHECK(COMPARE_STRINGS("value3", ini_cfg->GetValue("section2", "key1")));
  CHECK(COMPARE_STRINGS(""      , ini_cfg->GetValue("section2", "key2")));
  CHECK(COMPARE_STRINGS(""      , ini_cfg->GetValue("section3", "key1")));

  CHECK(COMPARE_STRINGS("", ini_cfg->GetValue("section1", "key11")));
  ini_cfg->SetValue("section1", "key11", "value11");
  CHECK(COMPARE_STRINGS("value11", ini_cfg->GetValue("section1", "key11")));

  ini_cfg->SetValue("section1", "key2", "value22");
  CHECK(COMPARE_STRINGS("value22", ini_cfg->GetValue("section1", "key2")));

  delete ini_cfg;
  ini_cfg = nullptr;

  ini_cfg = new IniConfig(cfg_path);
  CHECK(COMPARE_STRINGS("value1", ini_cfg->GetValue("section1", "key1")));
  CHECK(COMPARE_STRINGS("value22", ini_cfg->GetValue("section1", "key2")));
  CHECK(COMPARE_STRINGS(""      , ini_cfg->GetValue("section1", "key3")));
  CHECK(COMPARE_STRINGS("value3", ini_cfg->GetValue("section2", "key1")));
  CHECK(COMPARE_STRINGS(""      , ini_cfg->GetValue("section2", "key2")));
  CHECK(COMPARE_STRINGS(""      , ini_cfg->GetValue("section3", "key1")));
  CHECK(COMPARE_STRINGS("value11", ini_cfg->GetValue("section1", "key11")));
}
