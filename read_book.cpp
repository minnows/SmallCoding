#include "book.hpp"

int main(int ac, char** av)
{
    book mybook;
    mybook.Open();
    mybook.Read(10);
    mybook.Close();
}