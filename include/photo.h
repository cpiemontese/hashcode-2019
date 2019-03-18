#ifndef PHOTO_H
#define PHOTO_H

#include <string>

struct Photo {
    int id;
    std::string kind;
    int tag_num;
    std::string* tags;
};

#endif