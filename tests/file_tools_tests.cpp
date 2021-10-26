#include <doctest/doctest.h>

#include <binary_bakery_lib/file_tools.h>
#include <binary_bakery_testpaths.h>
using namespace bb;

TEST_CASE("directory_path")
{
   SUBCASE("must be a directory")
   {
      CHECK_THROWS_AS(directory_path{ "tests.cpp" }, std::exception);
      CHECK_NOTHROW(directory_path{ "." });
   }
   SUBCASE("must exist")
   {
      CHECK_THROWS_AS(directory_path{ "C:/doesnt_exist" }, std::exception);
   }
}


TEST_CASE("file_path")
{
   SUBCASE("must be a file")
   {
    CHECK_NOTHROW(file_path{testRoot / "tests.cpp"});
   }
}
