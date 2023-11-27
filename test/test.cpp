#include <stack>
#include <string>
#include <iostream>
#include <unordered_map>

using namespace std;

using T = std::stack<std::unordered_map<std::string, std::string>>;
using E = std::unordered_map<std::string, std::string>;

int main() {
   T t;
   t.push(E());
   E& e = t.top();
   e["10"] = "20";

   E v = t.top();
   std::cout << v["10"] << std::endl;

}