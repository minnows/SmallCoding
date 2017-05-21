#include <iostream> 
using namespace std;
class BaseX
{
public:
	BaseX()  {};
	~BaseX() {};
    virtual void WhoAmI()
	{
		cout << "I am A" << endl;
	}
};
