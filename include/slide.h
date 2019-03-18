#ifndef SLIDE_H
    #define SLIDE_H
#endif

#include <string>
#include <tuple>

enum struct SlideKind { H, V };

struct Slide {
    int tag_num;
    std::string* tags;
    SlideKind kind;
    union {
        int id;                     // SlideKind::H
        std::tuple<int, int> ids;   // SlideKind::V
    };
};