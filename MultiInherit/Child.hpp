#include <iostream> 
#include "BaseA.hpp"
#include "BaseB.hpp"
using namespace std;
class Child:public BaseA,public BaseB
{
public:
	Child()  {};
	~Child() {};
};
