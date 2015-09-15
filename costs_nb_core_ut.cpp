#define CATCH_CONFIG_MAIN
#include <vector>

#include "catch.hpp"

#include "costs_nb_core.h"

#define CREATE_LEAF_CATEGORY_R(VAR, ROOT, NAME, RATING) CategoriesElem* VAR = new CategoriesElem(NAME, ROOT, RATING)
#define CREATE_ROOT_CATEGORY_R(VAR, NAME, RATING) CREATE_LEAF_CATEGORY_R(VAR, nullptr, NAME, RATING)
#define CREATE_LEAF_CATEGORY(VAR, ROOT, NAME) CategoriesElem* VAR = new CategoriesElem(NAME, ROOT)
#define CREATE_ROOT_CATEGORY(VAR, NAME) CREATE_LEAF_CATEGORY(VAR, nullptr, NAME)
#define LINK_ROOT_LEAF(ROOT, VAR) ROOT->sub_categories.push_back(VAR)
#define ADD_SUB_CATEGORY(VAR, NAME) VAR->sub_categories.push_back(new CategoriesElem(NAME, VAR))
#define COMPARE_STRINGS(STR1, STR2) 0 == std::string(STR1).compare(STR2)
#define CREATE_EXPENSE(VAR, COST) CREATE_EXPENSE_D(VAR, Datetime(1, 1, 1, 1, 1, 1), COST)
#define CREATE_EXPENSE_D(VAR, DATE, COST) ExpenseElem VAR = ExpenseElem(ExpenseElem::DATE, COST)
#define LINK_EXPENSE(CAT, EXP) CAT->expenses.push_back(EXP);

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

extern std::string RemoveDisplaySubCategoriesPrefix(const std::string categoryName);
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

TEST_CASE( "MonthNumToStr", "[internal]" ) {
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

TEST_CASE( "MonthStrToNum", "[internal]" ) {
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

  CHECK(COMPARE_STRINGS("2012-Dec-01 05:38   1"  , e11_1.ToStr()));
  CHECK(COMPARE_STRINGS("1954-Jan-23 23:24   132", e11_2.ToStr()));
  CHECK(COMPARE_STRINGS("2174-Jul-30 11:01   56" , e11_3.ToStr()));
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

// TODO: void GetAllNestedExpenses(std::list<ExpenseElem> &expenses, const CategoriesElem* category);
