#include <iostream>

using namespace std;

extern int add(int, int);
extern int sub(int, int);

int main() { 
    int a = 10, b = 20;
    cout << "sub " << sub(a, b) << endl;
    cout << "add " << add(a, b) << endl;
}
