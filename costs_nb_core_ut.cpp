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
  CHECK( AddSubCategory(category, "Cat13"));
  auto subCat = category->subCategories.begin();
  CHECK(0 == std::string("Cat11").compare((*subCat)->categoryName));
  ++subCat;
  CHECK(0 == std::string("Cat12").compare((*subCat)->categoryName));
  ++subCat;
  CHECK(0 == std::string("Cat13").compare((*subCat)->categoryName));
  ++subCat;
  CHECK(subCat == category->subCategories.end());

  CHECK(!AddSubCategory(category, "Cat11"));
  subCat = category->subCategories.begin();
  CHECK(0 == std::string("Cat11").compare((*subCat)->categoryName));
  ++subCat;
  CHECK(0 == std::string("Cat12").compare((*subCat)->categoryName));
  ++subCat;
  CHECK(0 == std::string("Cat13").compare((*subCat)->categoryName));
  ++subCat;
  CHECK(subCat == category->subCategories.end());

  CHECK(!AddSubCategory(category, "Cat13"));
  subCat = category->subCategories.begin();
  CHECK(0 == std::string("Cat11").compare((*subCat)->categoryName));
  ++subCat;
  CHECK(0 == std::string("Cat12").compare((*subCat)->categoryName));
  ++subCat;
  CHECK(0 == std::string("Cat13").compare((*subCat)->categoryName));
  ++subCat;
  CHECK(subCat == category->subCategories.end());
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

