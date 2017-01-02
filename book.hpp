class book
{
    public:
        book();
        ~book();
        int GetBookStatus();
        void Open();
        void Close();
        void Read(int numOfPages);
        void Goto(int page);
        void AddTag(int page);
        int  GetTag();
    protected:
    private:
        int m_bookStatus;
        int m_currentPage;
        int m_tag;
};