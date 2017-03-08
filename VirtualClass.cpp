#include <iostream>

class VirtualClass
{
public:
	VirtualClass(){};
	~VirtualClass(){};
	virtual void PrintSelf()=0;
};

class ChildClass:public VirtualClass
{
public:
	void PrintSelf() { std::cout << "this is child class" << std::endl;};
};

class UserClass
{
public:
	UserClass(VirtualClass *virtualObj){m_virtualObj = virtualObj;};
private:
	VirtualClass *m_virtualObj;
};

int main()
{
	ChildClass child;
	UserClass user(&child);
	return 0;
}
