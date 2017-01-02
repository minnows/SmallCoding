#include "book.hpp"

book::book()
{
    m_bookStatus = 0;
}

book::~book()
{

}

int book::GetBookStatus()
{
    return m_bookStatus;
}

void book::Open()
{
    m_bookStatus = 1;
}

void book::Close()
{
    m_bookStatus = 0;
    m_currentPage = 0;
}

void book::Read(int numOfPages)
{
    //.......
    //
    m_currentPage += numOfPages;
    AddTag(m_currentPage);
    //
    //.......
}

void book::Goto(int page)
{
    m_currentPage = page;
}

void book::AddTag(int page)
{
    m_tag = page;
}

int book::GetTag()
{
    return m_tag;
}
