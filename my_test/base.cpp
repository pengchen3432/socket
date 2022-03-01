#include <iostream>
using namespace std;
class Base {
public: 
    Base(int a):b(a) {
        cout <<"构造函数"<<endl;
    };
    int b;
    Base(const Base &temp) {
        cout << "拷贝构造函数" <<endl;
        this->b = temp.b;
    }
    Base& operator+ (const Base &temp) {
        cout <<"赋值函数"<<endl;
        this->b += temp.b;
        return *this;
    }
};
int main()
{
    int a = 1;
    printf("0x%x\n", a);
    unsigned int b;
    b = (unsigned int)(a);
    printf("0x%08x\n", b);
    
}