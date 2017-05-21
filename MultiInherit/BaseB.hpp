#include <iostream> 
using namespace std;
class BaseB
{
public:
	BaseB()  {};
	~BaseB() {};
    virtual void WhoAmI()
	{
		cout << "I am B" << endl;
	}
};
