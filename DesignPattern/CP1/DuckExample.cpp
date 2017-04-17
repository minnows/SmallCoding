#include <iostream>
#include <stdio.h>

using namespace std;

class Duck
{
public:
    virtual  void quack()
    {
        cout << "quack quack" << endl;
    }

    virtual void display()
    {
        cout << "cannot display" << endl;
    }

protected:
};

class MallardDuck: public Duck
{
public:
    MallardDuck()
    {
    }

    void display()
    {
        cout << "i am a mallard duck" << endl;
    }
};

class RedheadDuck: public Duck
{
public:
    void RedHeadDuck()
    {
    }

    void display()
    {
        cout << "i am a readhead duck" << endl;
    }
};

class RubberDuck: public Duck
{
public:
    RubberDuck()
    {
    }

    void quack()
    {
        cout <<  "Bi Bi Bi" << endl;
    }

    void display()
    {
        cout << "i am a rubber duck" << endl;
    }
};

class WoodenDuck: public Duck
{
    void quack()
    {
    }

    void display()
    {    
        cout << "i am a wooden duck" << endl;
    }
};

int main()
{
    cout << "please input duck model " << endl;
    string duckType;
    getline(cin, duckType);
    Duck *duck;
    if (duckType == "MallardDuck")
    {
        duck = new MallardDuck();
    }
    else if (duckType == "RubberDuck")
    {
        duck = new RubberDuck();
    }
    else if (duckType == "RedheadDuck")
    {
        duck = new RedheadDuck();
    }
    else if (duckType == "WoodenDuck")
    {
        duck = new WoodenDuck();
    }
    else
    {
        cout << duckType << ":Duck doesn't exist" << endl;
        return 0;
    }
    cout << endl;
    duck->display();
    duck->quack();  
    return 1;
}


