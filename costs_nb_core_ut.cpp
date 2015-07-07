#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "costs_nb_core.h"

extern bool ValidateCategoryName(const std::string &newCategory);
TEST_CASE( "ValidateCategoryName", "[internal]" ) {
  CHECK(!ValidateCategoryName("") );
  CHECK( ValidateCategoryName("a") );
  CHECK( ValidateCategoryName("abc def") );
}

extern bool SubCategoriesContainCategory(const Categories_elem* categories, const std::string &categoryName);
TEST_CASE( "SubCategoriesContainCategory", "[internal]" ) {
  Categories_elem* category = new Categories_elem("Cat1", nullptr);
  category->subCategories.push_back(new Categories_elem("Cat11", category));
  category->subCategories.push_back(new Categories_elem("Cat12", category));
  category->subCategories.push_back(new Categories_elem("Cat13", category));

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

extern std::string AddDisplaySubCategoriesPrefix(const Categories_elem* category);
TEST_CASE( "AddDisplaySubCategoriesPrefix", "[internal]" ) {
  Categories_elem* category_w_subcat = new Categories_elem("Cat1", nullptr);
  category_w_subcat->subCategories.push_back(new Categories_elem("Cat11", category_w_subcat));
  CHECK(0 == std::string("> Cat1").compare(AddDisplaySubCategoriesPrefix(category_w_subcat)));

  Categories_elem* category_wo_subcat = new Categories_elem("Cat1", nullptr);
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

extern Categories_elem* GetSubCategoryByName(const Categories_elem* category, const std::string &categoryName);
TEST_CASE( "GetSubCategoryByName", "[internal]" ) {
  Categories_elem* category = new Categories_elem("Cat1", nullptr);
  Categories_elem* subCat1 = new Categories_elem("Cat11", category);
  Categories_elem* subCat2 = new Categories_elem("Cat12", category);
  Categories_elem* subCat3 = new Categories_elem("Cat13", category);
  category->subCategories.push_back(subCat1);
  category->subCategories.push_back(subCat2);
  category->subCategories.push_back(subCat3);

  CHECK(subCat1 == GetSubCategoryByName(category, "Cat11"));
  CHECK(subCat2 == GetSubCategoryByName(category, "Cat12"));
  CHECK(subCat3 == GetSubCategoryByName(category, "Cat13"));
  CHECK(nullptr == GetSubCategoryByName(category, "Cat14"));
}

extern bool AddSubCategory(Categories_elem* category, const std::string &newCategoryName);
TEST_CASE( "AddSubCategory", "[internal]" ) {
  Categories_elem* category = new Categories_elem("Cat1", nullptr);
  category->subCategories.push_back(new Categories_elem("Cat11", category));
  category->subCategories.push_back(new Categories_elem("Cat12", category));
  REQUIRE( AddSubCategory(category, "Cat13"));
  auto subCat = category->subCategories.begin();
  REQUIRE(0 == std::string("Cat11").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(0 == std::string("Cat12").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(0 == std::string("Cat13").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(subCat == category->subCategories.end());

  REQUIRE(!AddSubCategory(category, "Cat11"));
  subCat = category->subCategories.begin();
  REQUIRE(0 == std::string("Cat11").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(0 == std::string("Cat12").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(0 == std::string("Cat13").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(subCat == category->subCategories.end());

  REQUIRE(!AddSubCategory(category, "Cat13"));
  subCat = category->subCategories.begin();
  REQUIRE(0 == std::string("Cat11").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(0 == std::string("Cat12").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(0 == std::string("Cat13").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(subCat == category->subCategories.end());
}

extern std::string intMonth2str(unsigned int month);
TEST_CASE( "intMonth2str", "[internal]" ) {
  CHECK(0 == std::string("Jan").compare(intMonth2str( 1)));
  CHECK(0 == std::string("Feb").compare(intMonth2str( 2)));
  CHECK(0 == std::string("Mar").compare(intMonth2str( 3)));
  CHECK(0 == std::string("Apr").compare(intMonth2str( 4)));
  CHECK(0 == std::string("May").compare(intMonth2str( 5)));
  CHECK(0 == std::string("Jun").compare(intMonth2str( 6)));
  CHECK(0 == std::string("Jul").compare(intMonth2str( 7)));
  CHECK(0 == std::string("Aug").compare(intMonth2str( 8)));
  CHECK(0 == std::string("Sep").compare(intMonth2str( 9)));
  CHECK(0 == std::string("Oct").compare(intMonth2str(10)));
  CHECK(0 == std::string("Nov").compare(intMonth2str(11)));
  CHECK(0 == std::string("Dec").compare(intMonth2str(12)));
  CHECK(0 == std::string("---").compare(intMonth2str(13)));
  CHECK(0 == std::string("---").compare(intMonth2str( 0)));
}

extern bool RenameCategory_internal(Categories_elem* categories, const std::string &oldName0, const std::string &newName);
TEST_CASE( "RenameCategory", "[internal]" ) {
  Categories_elem* category = new Categories_elem("Cat1", nullptr);
  category->subCategories.push_back(new Categories_elem("Cat11", category));
  category->subCategories.push_back(new Categories_elem("Cat12", category));
  category->subCategories.push_back(new Categories_elem("Cat13", category));

  REQUIRE(!RenameCategory_internal(category, "Cat11", "Cat12"));
  REQUIRE(!RenameCategory_internal(category, "Cat12", "Cat12"));
  REQUIRE(!RenameCategory_internal(category, "Cat11", ""));
  auto subCat = category->subCategories.begin();
  REQUIRE(0 == std::string("Cat11").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(0 == std::string("Cat12").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(0 == std::string("Cat13").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(subCat == category->subCategories.end());

  REQUIRE( RenameCategory_internal(category, "Cat11", "Tac111"));
  REQUIRE( RenameCategory_internal(category, "Cat13", "Cat14"));
  REQUIRE( RenameCategory_internal(category, "Tac111", "Tac0"));
  subCat = category->subCategories.begin();
  REQUIRE(0 == std::string("Tac0").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(0 == std::string("Cat12").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(0 == std::string("Cat14").compare((*subCat)->categoryName));
  ++subCat;
  REQUIRE(subCat == category->subCategories.end());
}

extern std::list<std::string> GetExpenses_internal(Categories_elem* categories, const std::string &selectedCategory0);
TEST_CASE( "GetExpenses", "[internal]" ) {
  Categories_elem* category = new Categories_elem("Cat1", nullptr);
  Categories_elem* cat11 = new Categories_elem("Cat11", category);
  Categories_elem* cat12 = new Categories_elem("Cat12", category);
  category->subCategories.push_back(cat11);
  category->subCategories.push_back(cat12);

  Expense_elem e11_1 = Expense_elem("2015-Jul-01", 1);
  Expense_elem e11_2 = Expense_elem("2015-Sep-01", 132);
  Expense_elem e11_3 = Expense_elem("2015-Sep-17", 56);
  Expense_elem e12_1 = Expense_elem("2015-Sep-01", 463);
  Expense_elem e12_2 = Expense_elem("2015-Dec-29", 65);
  cat11->expenses.push_back(e11_1);
  cat11->expenses.push_back(e11_2);
  cat11->expenses.push_back(e11_3);
  cat12->expenses.push_back(e12_1);
  cat12->expenses.push_back(e12_2);

  auto Expenses1 = GetExpenses_internal(category, "Cat11");
  auto Expenses1i = Expenses1.begin();
  REQUIRE(0 == std::string(e11_1.toStr()).compare(*Expenses1i));
  ++Expenses1i;
  REQUIRE(0 == std::string(e11_2.toStr()).compare(*Expenses1i));
  ++Expenses1i;
  REQUIRE(0 == std::string(e11_3.toStr()).compare(*Expenses1i));
  ++Expenses1i;
  REQUIRE(Expenses1i == Expenses1.end());

  auto Expenses2 = GetExpenses_internal(category, "Cat12");
  auto Expenses2i = Expenses2.begin();
  REQUIRE(0 == std::string(e12_1.toStr()).compare(*Expenses2i));
  ++Expenses2i;
  REQUIRE(0 == std::string(e12_2.toStr()).compare(*Expenses2i));
  ++Expenses2i;
  REQUIRE(Expenses2i == Expenses2.end());

  auto Expenses0 = GetExpenses_internal(category, "Cat10");
  REQUIRE(Expenses0.empty());
}

TEST_CASE( "Expense_elem.toStr", "[internal]" ) {
  Expense_elem e11_1 = Expense_elem("2015-Jul-01", 1);
  Expense_elem e11_2 = Expense_elem("2015-Sep-01", 132);
  Expense_elem e11_3 = Expense_elem("2015-Sep-17", 56);

  CHECK(0 == std::string("2015-Jul-01   1")  .compare(e11_1.toStr()));
  CHECK(0 == std::string("2015-Sep-01   132").compare(e11_2.toStr()));
  CHECK(0 == std::string("2015-Sep-17   56") .compare(e11_3.toStr()));
}

extern std::list<std::string> GetAllExpenses_internal(Categories_elem* categories, const std::string &selectedCategory0);
TEST_CASE( "GetAllExpenses", "[internal]" ) {
  Categories_elem* category = new Categories_elem("Cat1", nullptr);
  Categories_elem* cat11 = new Categories_elem("Cat11", category);
  Categories_elem* cat12 = new Categories_elem("Cat12", category);
  Categories_elem* cat111 = new Categories_elem("Cat111", cat11);
  Categories_elem* cat112 = new Categories_elem("Cat112", cat11);
  Categories_elem* cat1111 = new Categories_elem("Cat1111", cat111);
  category->subCategories.push_back(cat11);
  category->subCategories.push_back(cat12);
  cat11->subCategories.push_back(cat111);
  cat11->subCategories.push_back(cat112);
  cat111->subCategories.push_back(cat1111);

  Expense_elem e1_1 = Expense_elem("DT1.1", 1);
  Expense_elem e1_2 = Expense_elem("DT1.2", 2);
  Expense_elem e11_1 = Expense_elem("DT11.1", 3);
  Expense_elem e11_2 = Expense_elem("DT11.2", 4);
  Expense_elem e12_1 = Expense_elem("DT12.1", 5);
  Expense_elem e111_1 = Expense_elem("DT111.1", 6);
  Expense_elem e112_1 = Expense_elem("DT112.1", 7);
  Expense_elem e112_2 = Expense_elem("DT112.2", 8);
  Expense_elem e1111_1 = Expense_elem("DT1111.1", 9);

  category->expenses.push_back(e1_1);
  category->expenses.push_back(e1_2);
  cat11->expenses.push_back(e11_1);
  cat11->expenses.push_back(e11_2);
  cat12->expenses.push_back(e12_1);
  cat111->expenses.push_back(e111_1);
  cat112->expenses.push_back(e112_1);
  cat112->expenses.push_back(e112_2);
  cat1111->expenses.push_back(e1111_1);

  auto Expenses = GetAllExpenses_internal(category, "Cat11");
  auto i = Expenses.begin();
  REQUIRE(0 == std::string(e11_1.toStr()).compare(*i));
  ++i;
  REQUIRE(0 == std::string(e11_2.toStr()).compare(*i));
  ++i;
  REQUIRE(0 == std::string(e111_1.toStr()).compare(*i));
  ++i;
  REQUIRE(0 == std::string(e1111_1.toStr()).compare(*i));
  ++i;
  REQUIRE(0 == std::string(e112_1.toStr()).compare(*i));
  ++i;
  REQUIRE(0 == std::string(e112_2.toStr()).compare(*i));
  ++i;
  REQUIRE(i == Expenses.end());
}
