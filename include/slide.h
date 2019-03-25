#ifndef SLIDE_H
#define SLIDE_H

#include <tuple>
#include <cstring>
#include <string>
#include <photo.h>

enum struct SlideKind { H, V };

struct Slide {
    int tag_num;
    std::string* tags;
    SlideKind kind;
    union Id_or_ids {
        int id;                     // SlideKind::H
        std::tuple<int, int> ids;   // SlideKind::V
        Id_or_ids() { memset(this, 0, sizeof(Id_or_ids)); }
    };
    Id_or_ids id_or_ids;
};

int copy_photo_to_hslide(Photo src, Slide& dst);
void copy(Slide src, Slide& dst);

#endif