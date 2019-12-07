#include <iostream>
#include <fstream>
#include <string>
#include <regex>

using namespace std;

int main() {
    // string test_string = "12 5 -3 5";

    // regex re("(-?\\d+)");
    // sregex_iterator next(test_string.begin(), test_string.end(), re);
    // sregex_iterator end;
    // ptrdiff_t match_count(distance(next, end));
    // cout << "All: " << match_count << endl;
    // while (next != end) {
    //     smatch match = *next++;
    //     int num = stoi(match.str());
    //     cout << num << endl;
    // }

    ifstream fin("test.txt");
    string inp_buf, tmp_buf;
    double num;

    regex re("([-.,\\w]+)");

    while (!fin.eof()) {
        getline(fin, inp_buf);
        cout << inp_buf << endl;
        if (inp_buf.empty()) {
            continue;
        }

        sregex_iterator next(inp_buf.begin(), inp_buf.end(), re);
        sregex_iterator end;
        while (next != end) {
            smatch match = *next++;
            cout << match.str() << ": ";
            try {
                num = stod(match.str());
                if (match.str().find(',') != string::npos) {
                    throw invalid_argument("");
                }
            }
            catch(invalid_argument excp) {
                cout << "not num: " <<  match.str() << endl;
                continue;
            }
            cout << num << endl;
        }
        cout << endl;
    }
    fin.close();

    return 0;
}