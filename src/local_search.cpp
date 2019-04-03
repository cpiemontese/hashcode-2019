#include <tuple>
#include <vector>
#include <math.h>
#include <random>
#include <iostream>
#include <functional>
#include <assert.h>
#include <slide.h>
#include <local_search.h>
using namespace std;

int score_fn(Slide& s1, Slide& s2);
int score_fn(Photo* v1, Photo* v2);
inline int score_array_dim(int size);
inline int transform_id(int size, int i, int j);
int get_score(int scores[], int i, int j, int size);
int compute_slideshow_score(vector<int> slideshow, vector<Slide> slides, int len);

inline int score_array_dim(int size) {
    return (size*size - size)/2;
}

int score_fn(Slide& s1, Slide& s2) {
    int common_tags = 0;
    for (int i = 0; i < s1.tag_num; i++) {
        for (int j = 0; j < s2.tag_num; j++) {
            if (s1.tags[i] == s2.tags[j])
                common_tags++;
        }
    }
    return min(common_tags, min(s1.tag_num - common_tags, s2.tag_num - common_tags));
}

int score_fn(Photo* v1, Photo* v2) {
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

void local_search_verticals(vector<Photo*> vphotos, int vphotos_len, int vslides_num, vector<Slide> vertical_slides) {
    vector<tuple<int, int>> tmp_slides(vslides_num);

    // use scores to create a starting set of slides greedily
    int slide_id = 0;
    int total_score = 0;
    vector<bool> selected(vphotos_len, false);
    for (int i = 0; i < vphotos_len && slide_id < vslides_num; i++) {
        int max = -1;
        int sel_id = -1;

        if (selected[i])    // i was already selected as part of a slide
            continue;

        for (int j = 0; j < vphotos_len; j++) {
            if (!selected[j]) {
                int s =  score_fn(vphotos[i], vphotos[j]);
                if (s > max) {
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

    cout << "Starting score: " << total_score << endl;

    int current_max_score = total_score;
    vector<tuple<int, int>> total_slides(vslides_num);
    for (int i = 0; i < vslides_num; i++)
        total_slides[i] = tmp_slides[i];

    for (int i = 0; i < MAX_ITER; i++) {
        cout << "\r" << "Iteration " << i << endl;
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
                rid1_old_score = score_fn(vphotos[id0], vphotos[id1]);
                rid1_other = (id0 == rid1) ? id1 : id0;
                done1 = true;
            }
            if (id0 == rid2 || id1 == rid2) {
                rid2_old_score = score_fn(vphotos[id0], vphotos[id1]);
                rid2_other = (id0 == rid2) ? id1 : id0;
                done2 = true;
            }
        }

        if (rid1_other == rid2 && rid2_other == rid1)
            continue;

        // if rid1/rid2 is not part of a pair then rid?_other will be -1 and the
        // new score will be 0
        rid1_new_score = (rid2_other != -1) ? score_fn(vphotos[rid1], vphotos[rid2_other]) : 0;
        rid2_new_score = (rid1_other != -1) ? score_fn(vphotos[rid2], vphotos[rid1_other]) : 0;

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
    }

    cout << "Final vertical score: " << total_score << endl;

    // create the actual vertical slides
    for (int i = 0; i < vslides_num; i++) {
        int id1 = get<0>(total_slides[i]);
        int id2 = get<1>(total_slides[i]);

        vertical_slides[i].kind = SlideKind::V;
        vertical_slides[i].id_or_ids.ids = make_tuple(vphotos[id1]->id, vphotos[id2]->id);
        vertical_slides[i].tag_num = score_fn(vphotos[id1], vphotos[id2]);
        //vertical_slides[i].tags = new string[vertical_slides[i].tag_num];

        // copy the tags from id1
        for (int j = 0; j < vphotos[id1]->tag_num; j++)
            vertical_slides[i].tags.push_back(vphotos[id1]->tags[j]);

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

int compute_slideshow_score(vector<int> slideshow, vector<Slide> slides, int len) {
    int score = 0;
    for (int i = 0; i < len - 1; i++)
        score += score_fn(slides[slideshow[i]], slides[slideshow[i + 1]]);

    return score;
}

void local_search_slides(vector<Slide> slides, int slides_len, vector<Slide> final_slides) {
    vector<int> tmp_slides(slides_len);

    // use scores to create a starting set of slides greedily
    int slide_id = 0;
    int total_score = 0;
    vector<bool> selected(slides_len, false);
    for (int i = 0; i < slides_len && slide_id < slides_len - 1; i++) {
        int max = 0;
        int sel_id = -1;

        if (selected[i])    // i was already selected as part of a slide
            continue;

        for (int j = 0; j < slides_len; j++) {
            int s = score_fn(slides[i], slides[j]);
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

    // if slides_len is odd then the last slide gets left out;
    if (slide_id != slides_len) {
        for (int i = 0; i < slides_len; i++) {
            if (!selected[i]) {
                tmp_slides[slides_len - 1] = i;
                break;
            }
        }
    }

    // iterate a fixed number of times to try to get a better solution
    const int MAX_ITER = 10000;

    random_device rd;
    mt19937 rng(rd());
    uniform_int_distribution<int> uni_int(0, slides_len - 1);
    uniform_real_distribution<double> uni_real(0.0, 1.0);

    cout << "Starting score: " << total_score << endl;

    int current_max_score = total_score;
    vector<int> total_slides(slides_len);
    for (int i = 0; i < slides_len; i++)
        total_slides[i] = tmp_slides[i];

    for (int i = 0; i < MAX_ITER; i++) {
        int rid1 = uni_int(rng);
        int rid2 = uni_int(rng);

        // be sure to generate a different id than rid1
        while(rid2 == rid1)
            rid2 = uni_int(rng);
        
        // swap
        swap(tmp_slides[rid1], tmp_slides[rid2]);
        int new_score = compute_slideshow_score(tmp_slides, slides, slides_len); // smart score update

        if (new_score > current_max_score || uni_real(rng) >= 0.5) {
            current_max_score = new_score;
            
            // if new_score is actually better than the best, save it
            if (new_score > total_score) {
                total_score = new_score;
                for (int i = 0; i < slides_len; i++)
                    total_slides[i] = tmp_slides[i];
            }
        } else
            swap(tmp_slides[rid1], tmp_slides[rid2]);
    }

    cout << "Final total score: " << total_score << endl;

    // create the actual vertical slides
    for (int i = 0; i < slides_len; i++)
        copy(slides[total_slides[i]], final_slides[i]);
}

// TODO: remove score_fn, overload score funcs
template <typename InType, typename OutType>
void local_search(vector<InType> input, int input_len, vector<OutType> output) {
    vector<int> tmp_solution(input_len);

    // use scores to create a starting set of slides greedily
    int current = 0;
    int total_score = 0;
    vector<bool> selected(input_len, false);
    for (int i = 0; i < input_len && current < input_len - 1; i++) {
        int max = 0;
        int sel_id = -1;

        if (selected[i])    // i was already selected as part of a slide
            continue;

        for (int j = 0; j < input_len; j++) {
            int s = score_fn(input[i], input[j]);
            if (s > max && !selected[j]) {
                max = s;
                sel_id = j;
            }
        }

        // here max will always be >= 0 because there is always a successor
        total_score += max;
        selected[i] = true;
        selected[sel_id] = true;
        tmp_solution[current] = i;
        tmp_solution[current + 1] = sel_id;
        current += 2;
    }

    // if input_len is odd then the last input would get left out
    if (current != input_len) {
        cout << "odd" << endl; // this shouldn't happen
        for (int i = 0; i < input_len; i++) {
            if (!selected[i]) {
                tmp_solution[input_len - 1] = i;
                break;
            }
        }
    }

    // iterate a fixed number of times to try to get a better solution
    const int MAX_ITER = 10000;

    random_device rd;
    mt19937 rng(rd());
    uniform_int_distribution<int> uni_int(0, input_len - 1);
    uniform_real_distribution<double> uni_real(0.0, 1.0);

    cout << "Starting score: " << total_score << endl;

    int current_max_score = total_score;
    vector<int> max_solution(input_len);
    for (int i = 0; i < input_len; i++)
        max_solution[i] = tmp_solution[i];

    for (int i = 0; i < MAX_ITER; i++) {
        cout << "\r" << "Iteration " << i << endl;
        int rid1 = uni_int(rng);
        int rid2 = uni_int(rng);

        // be sure to generate a different id than rid1
        while(rid2 == rid1)
            rid2 = uni_int(rng);

        // compute new score given swap
        int new_score = new_score_after_swap(current_max_score, rid1, rid2, tmp_solution, input_len, input);

        if (new_score > current_max_score || uni_real(rng) >= 0.5) {
            current_max_score = new_score;
            swap(tmp_solution[rid1], tmp_solution[rid2]);
            
            // if new_score is actually better than the best, save it
            if (new_score > total_score) {
                total_score = new_score;
                for (int i = 0; i < input_len; i++)
                    max_solution[i] = tmp_solution[i];
            }
        }
    }

    cout << "Final total score: " << total_score << endl;

    copy_final_result(max_solution, input, output, input_len);
}

// TODO: refactor
// new_score_after_swap for verticals
int new_score_after_swap(
    int current_max,
    int id0, int id1,
    vector<int> tmp_solution, int solution_len,
    vector<Photo*> vphotos
) {
    assert(solution_len % 2 == 0);
    int other0 = -1;
    int other1 = -1;
    int old_score0 = 0;
    int old_score1 = 0;
    bool done0 = false;
    bool done1 = false;
    for (int i = 0; i < solution_len - 1 && !(done0 && done1); i += 2) {
        int tmp_i0 = tmp_solution[i];
        int tmp_i1 = tmp_solution[i + 1];
        if (tmp_i0 == id0 || tmp_i1 == id0) {
            old_score0 = score_fn(vphotos[tmp_i0], vphotos[tmp_i1]);
            other0 = (id0 == tmp_i0) ? tmp_i1 : tmp_i0;
            done0 = true;
        }
        if (tmp_i0 == id1 || tmp_i1 == id1) {
            old_score1 = score_fn(vphotos[tmp_i0], vphotos[tmp_i1]);
            other1 = (id1 == tmp_i0) ? tmp_i1 : tmp_i0;
            done1 = true;
        }
    }

    // if id0 or id1 is not part of a pair then other0/1 will be -1 and the
    // new score will be 0
    int new_score0 = (other1 != -1) ? score_fn(vphotos[id0], vphotos[other1]) : 0;
    int new_score1 = (other0 != -1) ? score_fn(vphotos[id1], vphotos[other0]) : 0;

    // accept either a better score or a bad score but with probability 0.5
    return current_max - old_score0 - old_score1 + new_score0 + new_score1;
}

// new_score_after_swap for slides
int new_score_after_swap(
    int current_max,
    int id0, int id1,
    vector<int> tmp_solution, int solution_len,
    vector<Slide> slides
) {
    int id[2] = {id0, id1};
    int prev[2] = {-1, -1};
    int succ[2] = {-1, -1};
    int old_score[2] = {0, 0};
    int new_score0 = 0;
    int new_score1 = 0;
    bool done[2] = {false, false};

    for (int i = 0; i < solution_len && !(done[0] && done[1]); i++) {
        int sel = (id0 == i) ? 0 : ((id1 == i) ? 1 : -1);
        if (sel != -1) {
            if (id[sel] != 0) {
                prev[sel] = id[sel] - 1;
                old_score[sel] += score_fn(slides[prev[sel]], slides[id[sel]]);
            }
            if (id[sel] != solution_len) {
                succ[sel] = id[sel] + 1;
                old_score[sel] += score_fn(slides[id[sel]], slides[succ[sel]]);
            }
            done[sel] = true;
        }
    }

    new_score0 += (prev[1] != -1) ? score_fn(slides[prev[1]], slides[id[0]]) : 0;
    new_score0 += (succ[1] != -1) ? score_fn(slides[id[0]], slides[succ[1]]) : 0;
    new_score1 += (prev[0] != -1) ? score_fn(slides[prev[0]], slides[id[1]]) : 0;
    new_score1 += (succ[0] != -1) ? score_fn(slides[id[1]], slides[succ[0]]) : 0;

    return current_max - old_score[0] - old_score[1 + new_score0 + new_score1];
}

// TODO: complete
void copy_final_result(
    vector<int> max_solution,
    vector<Photo*> vphotos,
    vector<Slide> output,
    int len) {
    for (int i = 0; i < len - 1; i += 2) {
        int id0 = max_solution[i];
        int id1 = max_solution[i + 1];

        output[i].kind = SlideKind::V;
        output[i].id_or_ids.ids = make_tuple(vphotos[id0]->id, vphotos[id1]->id);
        output[i].tag_num = score_fn(vphotos[id0], vphotos[id1]);

        // copy the tags from id1
        for (int j = 0; j < vphotos[id0]->tag_num; j++)
            output[i].tags.push_back(vphotos[id0]->tags[j]);

        // copy the tags from id1 that do not appear in id1 also
        int current = vphotos[id0]->tag_num + 1;
        for (int j = 0; j < vphotos[id1]->tag_num && current < output[i].tag_num; j++) {
            bool present = false;
            for (int k = 0; k < vphotos[id0]->tag_num; k++)
                present = vphotos[id1]->tags[j] == output[i].tags[k];

            if (!present) {
                output[i].tags[current] = vphotos[id1]->tags[j];
                current++;
            }
        }
    }
}

void copy_final_result(
    vector<int> max_solution,
    vector<Slide> input,
    vector<Slide> output,
    int len) {
    for (int i = 0; i < len; i++)
        copy(input[max_solution[i]], output[i]);
}