#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "costs_nb_core.h"

extern bool ValidateCategoryName(const std::string &newCategory);
TEST_CASE( "ValidateCategoryName", "[internal]" ) {
  CHECK(!ValidateCategoryName("") );
  CHECK( ValidateCategoryName("a") );
  CHECK( ValidateCategoryName("abc def") );
}

extern bool SubCategoriesContainCategory(const CategoriesElem* categories, const std::string &categoryName);
TEST_CASE( "SubCategoriesContainCategory", "[internal]" ) {
  CategoriesElem* category = new CategoriesElem("Cat1", nullptr);
  category->sub_categories.push_back(new CategoriesElem("Cat11", category));
  category->sub_categories.push_back(new CategoriesElem("Cat12", category));
  category->sub_categories.push_back(new CategoriesElem("Cat13", category));

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
  CategoriesElem* category_w_subcat = new CategoriesElem("Cat1", nullptr);
  category_w_subcat->sub_categories.push_back(new CategoriesElem("Cat11", category_w_subcat));
  CHECK(0 == std::string("> Cat1").compare(AddDisplaySubCategoriesPrefix(category_w_subcat)));

  CategoriesElem* category_wo_subcat = new CategoriesElem("Cat1", nullptr);
  CHECK(0 == std::string("Cat1").compare(AddDisplaySubCategoriesPrefix(category_wo_subcat)));
}

extern std::string RemoveDisplaySubCategoriesPrefix(const std::string categoryName);
TEST_CASE( "RemoveDisplaySubCategoriesPrefix", "[internal]" ) {
  CHECK(0 == std::string("").compare(RemoveDisplaySubCategoriesPrefix("")));
  CHECK(0 == std::string(">").compare(RemoveDisplaySubCategoriesPrefix(">")));
  CHECK(0 == std::string("> ").compare(RemoveDisplaySubCategoriesPrefix("> ")));
  CHECK(0 == std::string("C").compare(RemoveDisplaySubCategoriesPrefix("> C")));
  CHECK(0 == std::string("Cat1").compare(RemoveDisplaySubCategoriesPrefix("Cat1")));
  CHECK(0 == std::string("Cat1").compare(RemoveDisplaySubCategoriesPrefix("> Cat1")));
  CHECK(0 == std::string("> Cat1").compare(RemoveDisplaySubCategoriesPrefix("> > Cat1")));
  CHECK(0 == std::string("Cat1> ").compare(RemoveDisplaySubCategoriesPrefix("Cat1> ")));
}

extern CategoriesElem* GetSubCategoryByName(const CategoriesElem* category, const std::string &categoryName);
TEST_CASE( "GetSubCategoryByName", "[internal]" ) {
  CategoriesElem* category = new CategoriesElem("Cat1", nullptr);
  CategoriesElem* subCat1 = new CategoriesElem("Cat11", category);
  CategoriesElem* subCat2 = new CategoriesElem("Cat12", category);
  CategoriesElem* subCat3 = new CategoriesElem("Cat13", category);
  category->sub_categories.push_back(subCat1);
  category->sub_categories.push_back(subCat2);
  category->sub_categories.push_back(subCat3);

  CHECK(subCat1 == GetSubCategoryByName(category, "Cat11"));
  CHECK(subCat2 == GetSubCategoryByName(category, "Cat12"));
  CHECK(subCat3 == GetSubCategoryByName(category, "Cat13"));
  CHECK(nullptr == GetSubCategoryByName(category, "Cat14"));
}

extern bool AddSubCategory(CategoriesElem* category, const std::string &newCategoryName);
TEST_CASE( "AddSubCategory", "[internal]" ) {
  CategoriesElem* category = new CategoriesElem("Cat1", nullptr);
  category->sub_categories.push_back(new CategoriesElem("Cat11", category));
  category->sub_categories.push_back(new CategoriesElem("Cat12", category));
  REQUIRE( AddSubCategory(category, "Cat13"));
  auto subCat = category->sub_categories.begin();
  REQUIRE(0 == std::string("Cat11").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(0 == std::string("Cat12").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(0 == std::string("Cat13").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(subCat == category->sub_categories.end());

  REQUIRE(!AddSubCategory(category, "Cat11"));
  subCat = category->sub_categories.begin();
  REQUIRE(0 == std::string("Cat11").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(0 == std::string("Cat12").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(0 == std::string("Cat13").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(subCat == category->sub_categories.end());

  REQUIRE(!AddSubCategory(category, "Cat13"));
  subCat = category->sub_categories.begin();
  REQUIRE(0 == std::string("Cat11").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(0 == std::string("Cat12").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(0 == std::string("Cat13").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(subCat == category->sub_categories.end());
}

TEST_CASE( "MonthNumToStr", "[internal]" ) {
  CHECK(0 == std::string("Jan").compare(utils::MonthNumToStr( 1)));
  CHECK(0 == std::string("Feb").compare(utils::MonthNumToStr( 2)));
  CHECK(0 == std::string("Mar").compare(utils::MonthNumToStr( 3)));
  CHECK(0 == std::string("Apr").compare(utils::MonthNumToStr( 4)));
  CHECK(0 == std::string("May").compare(utils::MonthNumToStr( 5)));
  CHECK(0 == std::string("Jun").compare(utils::MonthNumToStr( 6)));
  CHECK(0 == std::string("Jul").compare(utils::MonthNumToStr( 7)));
  CHECK(0 == std::string("Aug").compare(utils::MonthNumToStr( 8)));
  CHECK(0 == std::string("Sep").compare(utils::MonthNumToStr( 9)));
  CHECK(0 == std::string("Oct").compare(utils::MonthNumToStr(10)));
  CHECK(0 == std::string("Nov").compare(utils::MonthNumToStr(11)));
  CHECK(0 == std::string("Dec").compare(utils::MonthNumToStr(12)));
  CHECK(0 == std::string("---").compare(utils::MonthNumToStr(13)));
  CHECK(0 == std::string("---").compare(utils::MonthNumToStr( 0)));
}

TEST_CASE( "MonthStrToNum", "[internal]" ) {
  CHECK(utils::MonthStrToNum("xxx") ==  0);
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
  CategoriesElem* category = new CategoriesElem("Cat1", nullptr);
  category->sub_categories.push_back(new CategoriesElem("Cat11", category));
  category->sub_categories.push_back(new CategoriesElem("Cat12", category));
  category->sub_categories.push_back(new CategoriesElem("Cat13", category));

  REQUIRE(!RenameCategory(category, "Cat11", "Cat12"));
  REQUIRE(!RenameCategory(category, "Cat12", "Cat12"));
  REQUIRE(!RenameCategory(category, "Cat11", ""));
  auto subCat = category->sub_categories.begin();
  REQUIRE(0 == std::string("Cat11").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(0 == std::string("Cat12").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(0 == std::string("Cat13").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(subCat == category->sub_categories.end());

  REQUIRE( RenameCategory(category, "Cat11", "Tac111"));
  REQUIRE( RenameCategory(category, "Cat13", "Cat14"));
  REQUIRE( RenameCategory(category, "Tac111", "Tac0"));
  subCat = category->sub_categories.begin();
  REQUIRE(0 == std::string("Tac0").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(0 == std::string("Cat12").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(0 == std::string("Cat14").compare((*subCat)->category_name));
  ++subCat;
  REQUIRE(subCat == category->sub_categories.end());
}

extern std::list<std::string> GetExpenses(CategoriesElem* categories, const std::string &selectedCategory0);
TEST_CASE( "GetExpenses", "[internal]" ) {
  CategoriesElem* category = new CategoriesElem("Cat1", nullptr);
  CategoriesElem* cat11 = new CategoriesElem("Cat11", category);
  CategoriesElem* cat12 = new CategoriesElem("Cat12", category);
  category->sub_categories.push_back(cat11);
  category->sub_categories.push_back(cat12);

  ExpenseElem e11_1 = ExpenseElem(ExpenseElem::Datetime(), 1);
  ExpenseElem e11_2 = ExpenseElem(ExpenseElem::Datetime(), 132);
  ExpenseElem e11_3 = ExpenseElem(ExpenseElem::Datetime(), 56);
  ExpenseElem e12_1 = ExpenseElem(ExpenseElem::Datetime(), 463);
  ExpenseElem e12_2 = ExpenseElem(ExpenseElem::Datetime(), 65);
  cat11->expenses.push_back(e11_1);
  cat11->expenses.push_back(e11_2);
  cat11->expenses.push_back(e11_3);
  cat12->expenses.push_back(e12_1);
  cat12->expenses.push_back(e12_2);

  auto Expenses1 = GetExpenses(category, "Cat11");
  auto Expenses1i = Expenses1.begin();
  REQUIRE(0 == std::string(e11_1.ToStr()).compare(*Expenses1i));
  ++Expenses1i;
  REQUIRE(0 == std::string(e11_2.ToStr()).compare(*Expenses1i));
  ++Expenses1i;
  REQUIRE(0 == std::string(e11_3.ToStr()).compare(*Expenses1i));
  ++Expenses1i;
  REQUIRE(Expenses1i == Expenses1.end());

  auto Expenses2 = GetExpenses(category, "Cat12");
  auto Expenses2i = Expenses2.begin();
  REQUIRE(0 == std::string(e12_1.ToStr()).compare(*Expenses2i));
  ++Expenses2i;
  REQUIRE(0 == std::string(e12_2.ToStr()).compare(*Expenses2i));
  ++Expenses2i;
  REQUIRE(Expenses2i == Expenses2.end());

  auto Expenses0 = GetExpenses(category, "Cat10");
  REQUIRE(Expenses0.empty());
}

TEST_CASE( "ExpenseElem.toStr", "[internal]" ) {
  ExpenseElem e11_1 = ExpenseElem(ExpenseElem::Datetime(), 1);
  ExpenseElem e11_2 = ExpenseElem(ExpenseElem::Datetime(), 132);
  ExpenseElem e11_3 = ExpenseElem(ExpenseElem::Datetime(), 56);

  CHECK(0 == std::string("2000-Jan-01 00:00   1")  .compare(e11_1.ToStr()));
  CHECK(0 == std::string("2000-Jan-01 00:00   132").compare(e11_2.ToStr()));
  CHECK(0 == std::string("2000-Jan-01 00:00   56") .compare(e11_3.ToStr()));
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
  CategoriesElem* category = new CategoriesElem("Cat1", nullptr);
  CategoriesElem* cat11 = new CategoriesElem("Cat11", category);
  CategoriesElem* cat12 = new CategoriesElem("Cat12", category);
  CategoriesElem* cat111 = new CategoriesElem("Cat111", cat11);
  CategoriesElem* cat112 = new CategoriesElem("Cat112", cat11);
  CategoriesElem* cat1111 = new CategoriesElem("Cat1111", cat111);
  category->sub_categories.push_back(cat11);
  category->sub_categories.push_back(cat12);
  cat11->sub_categories.push_back(cat111);
  cat11->sub_categories.push_back(cat112);
  cat111->sub_categories.push_back(cat1111);

  ExpenseElem e1_1    = ExpenseElem(ExpenseElem::Datetime(2015,  1, 11,  1, 21, 0), 1);
  ExpenseElem e1_2    = ExpenseElem(ExpenseElem::Datetime(2015,  7,  5,  6, 19, 0), 2);
  ExpenseElem e11_1   = ExpenseElem(ExpenseElem::Datetime(2014,  2, 21, 13, 34, 0), 3);
  ExpenseElem e11_2   = ExpenseElem(ExpenseElem::Datetime(2015,  1,  4,  3, 35, 0), 4);
  ExpenseElem e12_1   = ExpenseElem(ExpenseElem::Datetime(2015,  4,  1,  9, 13, 0), 5);
  ExpenseElem e111_1  = ExpenseElem(ExpenseElem::Datetime(2017,  4,  6,  7, 56, 0), 6);
  ExpenseElem e112_1  = ExpenseElem(ExpenseElem::Datetime(2015,  1,  1,  1,  7, 0), 7);
  ExpenseElem e112_2  = ExpenseElem(ExpenseElem::Datetime(2017,  9, 27, 21,  1, 0), 8);
  ExpenseElem e1111_1 = ExpenseElem(ExpenseElem::Datetime(2015, 12,  1, 23,  2, 0), 9);

  category->expenses.push_back(e1_1);
  category->expenses.push_back(e1_2);
  cat11->expenses.push_back(e11_1);
  cat11->expenses.push_back(e11_2);
  cat12->expenses.push_back(e12_1);
  cat111->expenses.push_back(e111_1);
  cat112->expenses.push_back(e112_1);
  cat112->expenses.push_back(e112_2);
  cat1111->expenses.push_back(e1111_1);

  auto Expenses = GetAllExpenses(category, "Cat11");
  auto i = Expenses.begin();
  REQUIRE(0 == std::string(e112_2.ToStr()).compare(*i));
  ++i;
  REQUIRE(0 == std::string(e111_1.ToStr()).compare(*i));
  ++i;
  REQUIRE(0 == std::string(e1111_1.ToStr()).compare(*i));
  ++i;
  REQUIRE(0 == std::string(e11_2.ToStr()).compare(*i));
  ++i;
  REQUIRE(0 == std::string(e112_1.ToStr()).compare(*i));
  ++i;
  REQUIRE(0 == std::string(e11_1.ToStr()).compare(*i));
  ++i;
  REQUIRE(i == Expenses.end());
}

extern void Buy(CategoriesElem* categories, const std::string &selectedCategory, const unsigned int cost, const std::string &info);
TEST_CASE( "Buy.rating", "[internal]" ) {
  CategoriesElem* cat1 = new CategoriesElem("Cat1", nullptr, 2);
  CategoriesElem* cat11 = new CategoriesElem("Cat11", cat1, 15);
  CategoriesElem* cat111 = new CategoriesElem("Cat111", cat11, 4);
  CategoriesElem* cat1111 = new CategoriesElem("Cat1111", cat111, 9);

  cat1->sub_categories.push_back(cat11);
  cat11->sub_categories.push_back(cat111);
  cat111->sub_categories.push_back(cat1111);

  Buy(cat111, "Cat1111", 7, "");
  CHECK( 3 == cat1->rating);
  CHECK(16 == cat11->rating);
  CHECK( 5 == cat111->rating);
  CHECK(10 == cat1111->rating);
}
