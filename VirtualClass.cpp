#include <iostream>
// this is code used to learn virtual class
// this is a virtual class
class VirtualClass
{
public:
	VirtualClass(){};
	~VirtualClass(){};
	virtual void PrintSelf()=0;
};

// this is child class inherented from virtual class
class ChildClass:public VirtualClass
{
public:
	void PrintSelf() { std::cout << "this is child class" << std::endl;};
};

// this is user class who has a member pointer to virtual base class
// this member is initialized in instructor
class UserClass
{
public:
	UserClass(VirtualClass *virtualObj){m_virtualObj = virtualObj;};
	void UserPrint()
	{
		m_virtualObj->PrintSelf();
	};
private:
	VirtualClass *m_virtualObj;
};

// this is a test class with a member of child class object
class TestClass
{
public:
	TestClass(){user = new UserClass(&child);};
	~TestClass(){delete user;};
	void TestPrint()
	{
		user->UserPrint();
	};
private:
	ChildClass child;
	UserClass* user;
	int temp;
};

int main()
{
	TestClass test;
	test.TestPrint();
	return 0;
}
