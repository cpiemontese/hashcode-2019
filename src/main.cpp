#include <fstream>
#include <iostream>
#include <memory>
using namespace std;

struct Photo {
    string kind;
    int tag_num;
    string** tags;
};

int main(int argc, char** argv) {
    if (argc != 3) {
        cout << "Two args required, " << argc - 1 << " given." << endl;
        return -1;
    }

    ifstream infile(argv[1]);
    int lines;
    infile >> lines;

    Photo photos[lines];
    for (int l = 0; l < lines; l++) {
        infile >> photos[l].kind >> photos[l].tag_num;
        photos[l].tags = new string*[photos[l].tag_num];
        for (int t = 0; t < photos[l].tag_num; t++) {
            photos[l].tags[t] = new string;
            infile >> *photos[l].tags[t];
        }
    }

    return 0;
}