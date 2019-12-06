#include <iostream>
#include <regex>
#include <string>

using namespace std;

int main() {
    string test_string = "12 5 -3 5";

    regex re("(-?\\d+)");
    sregex_iterator next(test_string.begin(), test_string.end(), re);
    sregex_iterator end;
    ptrdiff_t match_count(distance(next, end));
    cout << "All: " << match_count << endl;
    while (next != end) {
        smatch match = *next++;
        int num = stoi(match.str());
        cout << num << endl;
    }

    return 0;
}