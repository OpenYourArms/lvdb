#include <iostream>
#include <ctime>
#include "src/Journal.h"
using namespace std;

int main() {
    std::cout << "Hello, World!" << std::endl;
    string fpth="some path";
    OPERATION_p opp={1,{"kk","vv"}};
    Journal journal(fpth);
    journal._write(opp);
    return 0;
}
