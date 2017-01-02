#include "CppUTest/TestHarness.h"
#include "book.hpp"

TEST_GROUP(FirstTestGroup)
{
    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(FirstTestGroup, AddTagForPage10)
{
    book myBook;
    myBook.AddTag(10);
    CHECK_EQUAL(10,myBook.GetTag());
}

TEST(FirstTestGroup, AddTagForPage100)
{
    book myBook;
    myBook.AddTag(100);
    CHECK_EQUAL(100,myBook.GetTag());
}


