#include <iostream>
using namespace std;
class A {
public:
    void do_some() {
        cout <<" do A " <<endl;
    }
    virtual void fun() {
        cout <<"this is A"<<endl;
    }
    virtual ~A() {
        cout <<" A go " <<endl;
    }
};

class B : public A {
public:
    void do_some() {
        cout <<" do B " <<endl;
    }
    virtual void fun() {
        cout <<"this is B"<<endl;
    }
};

class C : public A {
public:
    void do_some() {
        cout <<" do C " <<endl;
    }
    void test() {
        cout <<" test C " <<endl;
    }
    virtual void fun() {
        cout <<"this is C"<<endl;
    }
    virtual ~C() {
        cout <<"C go" <<endl;
    }
};
int main()
{
    
}