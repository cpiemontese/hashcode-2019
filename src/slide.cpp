#include <slide.h>

using namespace std;

int copy_photo_to_hslide(Photo src, Slide& dst) {
    if (src.kind != "H")
        return -1;
    else {
        dst.kind = SlideKind::H;
        dst.id_or_ids.id = src.id;
        dst.tag_num = src.tag_num;
        dst.tags = src.tags;
        return 1;
    }
}

void copy(Slide src, Slide& dst) {
    dst.kind = src.kind;
    if (dst.kind == SlideKind::H)
        dst.id_or_ids.id = src.id_or_ids.id;
    else
        dst.id_or_ids.ids = src.id_or_ids.ids;
    dst.tag_num = src.tag_num;
    dst.tags = src.tags;
}