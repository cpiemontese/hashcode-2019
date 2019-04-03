#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <vector>
#include <slide.h>
#include <photo.h>

using namespace std;

template <typename InType, typename OutType>
void local_search(vector<InType> input, int input_len, vector<OutType> output);

#include <local_search_impl.h>

#endif