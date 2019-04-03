#ifndef PHOTO_H
#define PHOTO_H

#include <string>
#include <vector>

using namespace std;

struct Photo {
    int id;
    string kind;
    int tag_num;
    vector<string> tags;
};

#endif