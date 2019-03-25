#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <slide.h>
#include <photo.h>

void local_search_verticals(Photo* vphotos[], int vphotos_len, int vslides_num, Slide vertical_slides[]);
void local_search_slides(Slide slides[], int slides_len, Slide final_slides[]);

#endif