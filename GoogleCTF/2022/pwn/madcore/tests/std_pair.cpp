#include <utility>
#include <string>
#include <vector>
#include <iostream>


using namespace std;


int main (int argc, char *argv[]) {
    pair<string, vector<int>> testPair = make_pair(string("ABCD"), vector<int>{1});
    cout << "First: " << testPair.first << "\n";
    return 0;
}
