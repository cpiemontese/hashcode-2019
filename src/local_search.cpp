#include <tuple>
#include <math.h>
#include <random>
#include <iostream>
#include <slide.h>
#include <local_search.h>
using namespace std;

int score_array_dim(int size);
int slide_score(Slide& s1, Slide& s2);
int vertical_score(Photo* v1, Photo* v2);
inline int transform_id(int size, int i, int j);
int get_score(int scores[], int i, int j, int size);
void local_search_verticals(Photo* vphotos[], int vphotos_len, int vslides_num, Slide vertical_slides[]);

int _score_array_dim(int size, int pres) {
    if (size < 2)
        return 0;
    else if (size == 2)
        return pres + 1;
    else
        return _score_array_dim(size - 1, pres + size - 1);
}

int score_array_dim(int size) {
    return _score_array_dim(size, 0);
}

int slide_score(Slide& s1, Slide& s2) {
    int common_tags = 0;
    for (int i = 0; i < s1.tag_num; i++) {
        for (int j = 0; j < s2.tag_num; j++) {
            if (s1.tags[i] == s2.tags[j])
                common_tags++;
        }
    }
    return min(common_tags, min(s1.tag_num - common_tags, s2.tag_num - common_tags));
}

int vertical_score(Photo* v1, Photo* v2) {
    int score = v1->tag_num + v2->tag_num;
    for (int i = 0; i < v1->tag_num; i++) {
        for (int j = 0; j < v2->tag_num; j++) {
            if (v1->tags[i] == v2->tags[j])
                score--;
        }
    }
    return score;
}

// upper triangular matrix indices to array index
inline int transform_id(int size, int i, int j) {
    return i * size + j - (i > 0) ? i * 3 : 1;
}

int get_score(int scores[], int i, int j, int size) {
    return (i < j) ? scores[transform_id(size, i, j)] : scores[transform_id(size, j, i)];
}

void local_search_verticals(Photo* vphotos[], int vphotos_len, int vslides_num, Slide vertical_slides[]) {
    tuple<int, int> tmp_slides[vslides_num];

    // pre-compute scores between all vertical photos
    int score_len = score_array_dim(vphotos_len);
    int scores[score_len];

    for (int i = 0; i < vphotos_len - 1; i++) {
        for (int j = i + 1; i < vphotos_len; j++) {
            int id = transform_id(vphotos_len, i, j); 
            scores[id] = vertical_score(vphotos[i], vphotos[j]);
        }
    }

    // use scores to create a starting set of slides greedily
    int slide_id = 0;
    int total_score = 0;
    bool selected[vphotos_len] = { false };
    for (int i = 0; i < vphotos_len; i++) {
        int max = -1;
        int sel_id = -1;

        if (selected[i])    // i was already selected as part of a slide
            continue;

        for (int j = 0; j < vphotos_len; j++) {
            if (i != j) {
                int s = get_score(scores, i, j, vphotos_len);
                if (s > max && !selected[j]) {
                    max = s;
                    sel_id = j;
                }
            }
        }
        
        if (max != -1) {
            total_score += max;         // update score with score of selected pair
            selected[i] = true;         // i is selected
            selected[sel_id] = true;
            get<0>(tmp_slides[slide_id]) = i;
            get<1>(tmp_slides[slide_id]) = sel_id;
            slide_id++;
        }
        // if max is -1 then there is an odd number of vertical photos and one
        // photo is left unpaired
    }

    // iterate a fixed number of times to try to get a better solution
    const int MAX_ITER = 10000;

    random_device rd;
    mt19937 rng(rd());
    uniform_int_distribution<int> uni_int(0, vphotos_len - 1);
    uniform_real_distribution<double> uni_real(0.0, 1.0);

    for (int i = 0; i < MAX_ITER; i++) {
        int rid1 = uni_int(rng);
        int rid2 = uni_int(rng);

        // be sure to generate a different id than rid1
        while(rid2 == rid1)
            rid2 = uni_int(rng);
        
        int rid1_other = -1;
        int rid2_other = -1;
        int rid1_new_score = 0;
        int rid2_new_score = 0;
        int rid1_old_score = 0;
        int rid2_old_score = 0;
        bool done1 = false;
        bool done2 = false;

        // search for old scores and the ids of the other components of the pairs
        // that rid1 and rid2 are part of (if they are)
        for (int j = 0; j < vslides_num && !(done1 && done2); j++) {
            if (get<0>(tmp_slides[j]) == rid1 || get<1>(tmp_slides[j]) == rid1) {
                rid1_old_score = get_score(scores, get<0>(tmp_slides[j]), get<1>(tmp_slides[j]), vphotos_len);
                rid1_other = (get<0>(tmp_slides[j]) == rid1) ? get<1>(tmp_slides[j]) : get<0>(tmp_slides[j]);
                done1 = true;
            } else if (get<0>(tmp_slides[j]) == rid2 || get<1>(tmp_slides[j]) == rid2) {
                rid2_old_score = get_score(scores, get<0>(tmp_slides[j]), get<1>(tmp_slides[j]), vphotos_len);
                rid2_other = (get<0>(tmp_slides[j]) == rid2) ? get<1>(tmp_slides[j]) : get<0>(tmp_slides[j]);
                done2 = true;
            }
        }

        // if rid1/rid2 is not part of a pair then rid?_other will be -1 and the
        // new score will be 0
        rid1_new_score = (rid2_other != -1) ? get_score(scores, rid1, rid2_other, vphotos_len) : 0;
        rid2_new_score = (rid1_other != -1) ? get_score(scores, rid2, rid1_other, vphotos_len) : 0;

        // accept either a better score or a bad score but with probability 0.5
        int new_score = total_score - rid1_old_score - rid2_old_score + rid1_new_score + rid2_new_score;
        if (new_score > total_score || uni_real(rng) >= 0.5) {
            total_score = new_score;
            // update pairs
            bool done1 = rid1_other == -1;
            bool done2 = rid2_other == -1;
            for (int j = 0; j < vslides_num && !(done1 && done2); j++) {
                if (!done1 && (get<0>(tmp_slides[j]) == rid1 || get<1>(tmp_slides[j]) == rid1)) {
                    tmp_slides[j] = make_tuple(rid2, rid1_other);
                    done1 = true;
                }
                if (!done2 && (get<0>(tmp_slides[j]) == rid2 || get<1>(tmp_slides[j]) == rid2)) {
                    tmp_slides[j] = make_tuple(rid1, rid2_other);
                    done2 = true;
                }
            }
        }
    }

    cout << "Final vertical score: " << total_score;

    // create the actual vertical slides
    for (int i = 0; i < vslides_num; i++) {
        vertical_slides[i].kind = SlideKind::V;
        vertical_slides[i].id_or_ids.ids = tmp_slides[i];
        int id1 = get<0>(vertical_slides[i].id_or_ids.ids);
        int id2 = get<1>(vertical_slides[i].id_or_ids.ids);
        vertical_slides[i].tag_num = get_score(scores, id1, id2, vphotos_len);
        vertical_slides[i].tags = new string[vertical_slides[i].tag_num];

        // copy the tags from id1
        for (int j = 0; j < vphotos[id1]->tag_num; j++)
            vertical_slides[i].tags[j] = vphotos[id1]->tags[j];

        // copy the tags from id2 that do not appear in id1 also
        int current = vphotos[id1]->tag_num + 1;
        for (int j = 0; j < vphotos[id2]->tag_num && current < vertical_slides[i].tag_num; j++) {
            bool present = false;
            for (int k = 0; k < vphotos[id1]->tag_num; k++)
                present = vphotos[id2]->tags[j] == vertical_slides[i].tags[k];

            if (!present) {
                vertical_slides[i].tags[current] = vphotos[id2]->tags[j];
                current++;
            }
        }
    }
}