#include <tuple>
#include <math.h>
#include <random>
#include <iostream>
#include <assert.h>
#include <slide.h>
#include <local_search.h>
using namespace std;

inline int score_array_dim(int size);
int slide_score(Slide& s1, Slide& s2);
int vertical_score(Photo* v1, Photo* v2);
inline int transform_id(int size, int i, int j);
int get_score(int scores[], int i, int j, int size);
int compute_slideshow_score(int scores[], int slideshow[], int len);
void local_search_verticals(Photo* vphotos[], int vphotos_len, int vslides_num, Slide vertical_slides[]);
void local_search_slides(Slide* photos[], int photos_len, Slide final_slides[]);

inline int score_array_dim(int size) {
    return (size*size - size)/2;
}

int slide_score(Slide* s1, Slide* s2) {
    int common_tags = 0;
    for (int i = 0; i < s1->tag_num; i++) {
        for (int j = 0; j < s2->tag_num; j++) {
            if (s1->tags[i] == s2->tags[j])
                common_tags++;
        }
    }
    return min(common_tags, min(s1->tag_num - common_tags, s2->tag_num - common_tags));
}

int vertical_score(Photo* v1, Photo* v2) {
    if (v1->id == v2->id)
        return 0;

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
    return i*size + j - ((i + 1) * (i + 2) / 2);
}

int get_score(int scores[], int i, int j, int size) {
    if (i == j)
        return 0;
    else
        return (i < j) ? scores[transform_id(size, i, j)] : scores[transform_id(size, j, i)];
}

void local_search_verticals(Photo* vphotos[], int vphotos_len, int vslides_num, Slide vertical_slides[]) {
    tuple<int, int> tmp_slides[vslides_num];

    // pre-compute scores between all vertical photos
    int score_len = score_array_dim(vphotos_len);
    int scores[score_len];

    for (int i = 0; i < vphotos_len - 1; i++) {
        for (int j = i + 1; j < vphotos_len; j++) {
            int id = transform_id(vphotos_len, i, j); 
            scores[id] = vertical_score(vphotos[i], vphotos[j]);
            assert (get_score(scores, i, j, vphotos_len) == scores[id]);
        }
    }

    // use scores to create a starting set of slides greedily
    int slide_id = 0;
    int total_score = 0;
    bool selected[vphotos_len] = { false };
    for (int i = 0; i < vphotos_len && slide_id < vslides_num; i++) {
        int max = -1;
        int sel_id = -1;

        if (selected[i])    // i was already selected as part of a slide
            continue;

        for (int j = 0; j < vphotos_len; j++) {
            int s = get_score(scores, i, j, vphotos_len);
            if (s > max && !selected[j]) {
                max = s;
                sel_id = j;
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

    cout << "Starting score: " << total_score << endl;

    int current_max_score = total_score;
    tuple<int, int> total_slides[vslides_num];
    for (int i = 0; i < vslides_num; i++)
        total_slides[i] = tmp_slides[i];

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
            int id0 = get<0>(tmp_slides[j]);
            int id1 = get<1>(tmp_slides[j]);
            if (id0 == rid1 || id1 == rid1) {
                rid1_old_score = get_score(scores, id0, id1, vphotos_len);
                rid1_other = (id0 == rid1) ? id1 : id0;
                done1 = true;
            }
            if (id0 == rid2 || id1 == rid2) {
                rid2_old_score = get_score(scores, id0, id1, vphotos_len);
                rid2_other = (id0 == rid2) ? id1 : id0;
                done2 = true;
            }
        }

        if (rid1_other == rid2 && rid2_other == rid1)
            continue;

        // if rid1/rid2 is not part of a pair then rid?_other will be -1 and the
        // new score will be 0
        rid1_new_score = (rid2_other != -1) ? get_score(scores, rid1, rid2_other, vphotos_len) : 0;
        rid2_new_score = (rid1_other != -1) ? get_score(scores, rid2, rid1_other, vphotos_len) : 0;

        // accept either a better score or a bad score but with probability 0.5
        int new_score = current_max_score - rid1_old_score - rid2_old_score + rid1_new_score + rid2_new_score;
        if (new_score > current_max_score || uni_real(rng) >= 0.5) {
            current_max_score = new_score;
            
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

            // if new_score is actually better than the best, save it
            if (new_score > total_score) {
                total_score = new_score;
                for (int i = 0; i < vslides_num; i++)
                    total_slides[i] = tmp_slides[i];
            }
        }
        cout << "Total: " << total_score << ", current max: " << current_max_score << endl;
    }

    cout << "Final vertical score: " << total_score << endl;

    // create the actual vertical slides
    for (int i = 0; i < vslides_num; i++) {
        int id1 = get<0>(total_slides[i]);
        int id2 = get<1>(total_slides[i]);

        vertical_slides[i].kind = SlideKind::V;
        vertical_slides[i].id_or_ids.ids = make_tuple(vphotos[id1]->id, vphotos[id2]->id);
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

int compute_slideshow_score(int scores[], int slideshow[], int len) {
    int score = 0;
    for (int i = 0; i < len - 1; i++) {
        score += get_score(scores, i, i + 1, len);
    }
    return score;
}

void local_search_slides(Slide* photos[], int photos_len, Slide final_slides[]) {
    int tmp_slides[photos_len];

    // pre-compute scores between all vertical photos
    int score_len = score_array_dim(photos_len);
    int scores[score_len];

    for (int i = 0; i < photos_len - 1; i++) {
        for (int j = i + 1; j < photos_len; j++) {
            int id = transform_id(photos_len, i, j); 
            scores[id] = slide_score(photos[i], photos[j]);
            assert (get_score(scores, i, j, photos_len) == scores[id]);
        }
    }

    // use scores to create a starting set of slides greedily
    int slide_id = 0;
    int total_score = 0;
    bool selected[photos_len] = { false };

    for (int i = 0; i < photos_len && slide_id < photos_len - 1; i++) {
        int max = 0;
        int sel_id = -1;

        if (selected[i])    // i was already selected as part of a slide
            continue;

        for (int j = 0; j < photos_len; j++) {
            int s = get_score(scores, i, j, photos_len);
            if (s > max && !selected[j]) {
                max = s;
                sel_id = j;
            }
        }

        // here max will always be >= 0 because there is always a successor
        total_score += max;
        selected[i] = true;
        selected[sel_id] = true;
        tmp_slides[slide_id] = i;
        tmp_slides[slide_id + 1] = sel_id;
        slide_id += 2;
    } 

    // if photos_len is odd then the last slide gets left out;
    if (slide_id != photos_len) {
        for (int i = 0; i < photos_len; i++) {
            if (!selected[i]) {
                tmp_slides[photos_len - 1] = i;
                break;
            }
        }
    }

    // iterate a fixed number of times to try to get a better solution
    const int MAX_ITER = 10000;

    random_device rd;
    mt19937 rng(rd());
    uniform_int_distribution<int> uni_int(0, photos_len - 1);
    uniform_real_distribution<double> uni_real(0.0, 1.0);

    cout << "Starting score: " << total_score << endl;

    int current_max_score = total_score;
    int total_slides[photos_len];
    for (int i = 0; i < photos_len; i++)
        total_slides[i] = tmp_slides[i];

    int tmp;
    for (int i = 0; i < MAX_ITER; i++) {
        int rid1 = uni_int(rng);
        int rid2 = uni_int(rng);

        // be sure to generate a different id than rid1
        while(rid2 == rid1)
            rid2 = uni_int(rng);
        
        // swap
        swap(tmp_slides[rid1], tmp_slides[rid2]);
        int new_score = compute_slideshow_score(scores, tmp_slides, photos_len);

        if (new_score > current_max_score || uni_real(rng) >= 0.5) {
            current_max_score = new_score;
            
            // if new_score is actually better than the best, save it
            if (new_score > total_score) {
                total_score = new_score;
                for (int i = 0; i < photos_len; i++)
                    total_slides[i] = tmp_slides[i];
            }
        } else
            swap(tmp_slides[rid1], tmp_slides[rid2]);

        cout << "Total: " << total_score << ", current max: " << current_max_score << endl;
    }

    cout << "Final vertical score: " << total_score << endl;

    // create the actual vertical slides
    for (int i = 0; i < photos_len; i++) {
        final_slides[i].kind = photos[tmp_slides[i]]->kind;
        if (final_slides[i].kind == SlideKind::H)
            final_slides[i].id_or_ids.id = photos[tmp_slides[i]]->id_or_ids.id;
        else
            final_slides[i].id_or_ids.ids = photos[tmp_slides[i]]->id_or_ids.ids;
        final_slides[i].tag_num = photos[tmp_slides[i]]->tag_num;
        final_slides[i].tags = photos[tmp_slides[i]]->tags;
    }
}