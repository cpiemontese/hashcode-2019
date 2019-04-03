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

// new_score_after_swap for verticals
int new_score_after_swap(int current_max, int id0, int id1, vector<int> tmp_solution, int solution_len, vector<Photo*> vphotos) {
    assert(solution_len % 2 == 0);
    int id[2] = {id0, id1};
    int other[2] = {-1, -1};
    int old_score[2] = {-1, -1};
    int done[2] = {false, false};

    for (int i = 0; i < solution_len - 1 && !(done[0] && done[1]); i += 2) {
        int tmp_i0 = tmp_solution[i];
        int tmp_i1 = tmp_solution[i + 1];
        int sel = (id0 == tmp_i0 || id0 == tmp_i1) ? 0 : ((id1 == tmp_i0 || id1 == tmp_i1) ? 1 : -1);
        if (sel != -1) {
            old_score[sel] = score_fn(vphotos[tmp_i0], vphotos[tmp_i1]); 
            other[sel] = (tmp_i0 == id[sel]) ? tmp_i1 : tmp_i0;
            done[sel] = true;
        }
    }

    // if id0 or id1 is not part of a pair then other0/1 will be -1 and the
    // new score will be 0
    int new_score0 = (other[1] != -1) ? score_fn(vphotos[id0], vphotos[other[1]]) : 0;
    int new_score1 = (other[0] != -1) ? score_fn(vphotos[id1], vphotos[other[0]]) : 0;

    // accept either a better score or a bad score but with probability 0.5
    return current_max - old_score[0] - old_score[1] + new_score0 + new_score1;
}

// new_score_after_swap for slides
int new_score_after_swap(int current_max, int id0, int id1, vector<int> tmp_solution, int solution_len, vector<Slide> slides) {
    int id[2] = {id0, id1};
    int prev[2] = {-1, -1};
    int succ[2] = {-1, -1};
    int old_score[2] = {0, 0};
    int new_score0 = 0;
    int new_score1 = 0;
    bool done[2] = {false, false};


    for (int i = 0; i < solution_len && !(done[0] && done[1]); i++) {
        int sel = (id0 == tmp_solution[i]) ? 0 : ((id1 == tmp_solution[i]) ? 1 : -1);
        if (sel != -1) {
            int current = tmp_solution[i];
            if (id[sel] != 0) {
                prev[sel] = id[sel] - 1;
                old_score[sel] += score_fn(slides[tmp_solution[prev[sel]]], slides[current]);
            }
            if (id[sel] != solution_len - 1) {
                succ[sel] = id[sel] + 1;
                old_score[sel] += score_fn(slides[current], slides[tmp_solution[succ[sel]]]);
            }
            done[sel] = true;
        }
    }

    new_score0 += (prev[1] != -1) ? score_fn(slides[prev[1]], slides[id[0]]) : 0;
    new_score0 += (succ[1] != -1) ? score_fn(slides[id[0]], slides[succ[1]]) : 0;
    new_score1 += (prev[0] != -1) ? score_fn(slides[prev[0]], slides[id[1]]) : 0;
    new_score1 += (succ[0] != -1) ? score_fn(slides[id[1]], slides[succ[0]]) : 0;

    return current_max - old_score[0] - old_score[1] + new_score0 + new_score1;
}

void copy_final_result(vector<int> max_solution, vector<Photo*> vphotos, vector<Slide> output, int len) {
    for (int i = 0, out_id = 0; i < len - 1; i += 2, out_id++) {
        int id0 = max_solution[i];
        int id1 = max_solution[i + 1];

        output[out_id].kind = SlideKind::V;
        output[out_id].id_or_ids.ids = make_tuple(vphotos[id0]->id, vphotos[id1]->id);
        output[out_id].tag_num = score_fn(vphotos[id0], vphotos[id1]);

        // copy the tags from id1
        for (int j = 0; j < vphotos[id0]->tag_num; j++)
            output[out_id].tags.push_back(vphotos[id0]->tags[j]);

        // copy the tags from id1 that do not appear in id1 also
        for (int j = 0; j < vphotos[id1]->tag_num; j++) {
            bool present = false;
            for (int k = 0; k < vphotos[id0]->tag_num; k++)
                present = vphotos[id1]->tags[j] == output[out_id].tags[k];

            if (!present)
                output[out_id].tags.push_back(vphotos[id1]->tags[j]);
        }
    }
}

void copy_final_result(vector<int> max_solution, vector<Slide> input, vector<Slide> output, int len) {
    for (int i = 0; i < len; i++)
        copy(input[max_solution[i]], output[i]);
}