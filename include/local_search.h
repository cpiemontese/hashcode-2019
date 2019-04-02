#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <vector>
#include <slide.h>
#include <photo.h>

using namespace std;

void local_search_verticals(vector<Photo*> vphotos, int vphotos_len, int vslides_num, vector<Slide> vertical_slides);
void local_search_slides(vector<Slide> slides, int slides_len, vector<Slide> final_slides);

#endif