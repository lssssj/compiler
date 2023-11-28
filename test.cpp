#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdint.h>
#include <vector>

class BB {
    public:
        std::vector<int> v;
};

int main() {

    BB *b = new BB();
    std::reverse(b->v.begin(), b->v.end());
    std::cout << b->v.size() << std::endl;
    return 0;
}