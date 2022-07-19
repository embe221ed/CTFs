#include <utility>
#include <string>
#include <vector>
#include <iostream>
#include <optional>


using namespace std;


int main (int argc, char *argv[]) {
    ulong test = 0xf000000000000000;
    printf("0x%lx vs 0x%lx\n", test, test<<3);
    optional<int> otest;
    printf("%d\n", otest.has_value());
    return 0;
}
