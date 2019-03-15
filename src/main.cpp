#include <fstream>
#include <iostream>
#include <vector>
#include <math.h>
using namespace std;

struct Photo {
    int id;
    string kind;
    int tag_num;
    string* tags;
};

enum struct SlideKind { H, V };

struct Slide {
    int tag_num;
    string* tags;
    SlideKind kind;
    union {
        int id;                 // SlideKind::H
        pair<int, int> ids;     // SlideKind::V
    };
};

int slide_score(Slide& s1, Slide& s2);
int vertical_score(Photo& v1, Photo& v2);
Slide* local_search_verticals(Photo* vphotos[], int vphotos_len, int vslides_num);

int main(int argc, char** argv) {
    if (argc != 3) {
        cout << "Two args required, " << argc - 1 << " given." << endl;
        return -1;
    }

    ifstream infile(argv[1]);
    int lines;
    infile >> lines;

    // foto orizzontali -> gia' delle slides
    // foto verticali -> le separo, faccio pre-processing
    // dopo il pre-processing delle verticali creo un array t.c. id della slide
    // e' la posizione occupata nell'array, in modo da poter shufflare in
    // maniera banale per creare nuove soluzioni, i.e. lavoro sugli indici

    int vertical_photos = 0;
    Photo photos[lines];
    for (int l = 0; l < lines; l++) {
        infile >> photos[l].kind;
        infile >> photos[l].tag_num;

        if (photos[l].kind == "V")
            vertical_photos++;

        photos[l].id = l;
        photos[l].tags = new string[photos[l].tag_num];
        for (int t = 0; t < photos[l].tag_num; t++) {
            infile >> photos[l].tags[t];
        }
    }

    int v = 0;
    int vslides_num = floor(vertical_photos/2);
    cout << "total: " << vertical_photos << ", slides: " << vslides_num;
    Photo* vphotos[vertical_photos];
    for (int l = 0; l < lines; l++) {
        if (photos[l].kind == "V") {
            vphotos[v] = &photos[l];
            v++;
        }
    }

    // vertical_slides = local_search_verticals(verticals)
    // aggiungi vertical_slides a slides
    // slides = local_search_whole(slides)
    // produci out scorrendo le slides i.e. come sono ordinate e' l'output

    return 0;
}

int slide_score(Slide& s1, Slide& s2) {
    int score = 0;
    int common_tags = 0;
    for (int i = 0; i < s1.tag_num; i++) {
        for (int j = 0; j < s2.tag_num; j++) {
            if (s1.tags[i] == s2.tags[j])
                common_tags++;
        }
    }
    return min(common_tags, min(s1.tag_num - common_tags, s2.tag_num - common_tags));
}

int vertical_score(Photo& v1, Photo& v2) {
    int score = v1.tag_num + v2.tag_num;
    for (int i = 0; i < v1.tag_num; i++) {
        for (int j = 0; j < v2.tag_num; j++) {
            if (v1.tags[i] == v2.tags[j])
                score--;
        }
    }
    return score;
}

Slide* local_search_verticals(Photo* vphotos[], int vphotos_len, int vslides_num) {
    Slide vertical_slides[vslides_num];
    // pre-compute scores between all vertical photos
    // use scores to create a starting set of slides
    return vertical_slides;
}