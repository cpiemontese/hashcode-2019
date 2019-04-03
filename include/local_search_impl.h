#ifndef LOCAL_SEARCH_IMPL_H
#define LOCAL_SEARCH_IMPL_H

#include <vector>
#include <slide.h>
#include <photo.h>
#include <local_search.h>

using namespace std;

int score_fn(Slide& s1, Slide& s2);
int score_fn(Photo* v1, Photo* v2);
int new_score_after_swap(int current_max, int id0, int id1, vector<int> tmp_solution, int solution_len, vector<Photo*> vphotos);
int new_score_after_swap(int current_max, int id0, int id1, vector<int> tmp_solution, int solution_len, vector<Slide> slides);
void copy_final_result(vector<int> max_solution, vector<Photo*> vphotos, vector<Slide> output, int len);
void copy_final_result(vector<int> max_solution, vector<Slide> input, vector<Slide> output, int len);


template <typename InType, typename OutType>
void local_search(vector<InType> input, int input_len, vector<OutType> output) {
    vector<int> tmp_solution(input_len);

    // use scores to create a starting set of slides greedily
    int current = 0;
    int total_score = 0;
    for (int i = 0; i < input_len; i++) {
        tmp_solution[i] = i;
        total_score += (i > 0) ? score_fn(input[i], input[i - 1]) : 0;
    }

    // iterate a fixed number of times to try to get a better solution
    const int MAX_ITER = 100000;

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

#endif