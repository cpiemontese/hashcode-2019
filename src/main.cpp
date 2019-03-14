#include <fstream>
#include <iostream>
//#include <memory>
#include <vector>
using namespace std;

struct Photo {
    int id;
    int tag_num;
    string** tags;
};

enum struct SlideKind { H, V };

struct Slide {
    SlideKind kind;
    union {
        Photo horizontal_photo;                 // SlideKind::H
        pair<Photo, Photo> vertical_photos;     // SlideKind::V
    };
};

int slide_score(Slide s1, Slide s2) {
    // trova tag in comune
    // calcola # tag in comune
    // calcola # tag in s1 e NON in s2 i.e. tag s1 != tag comune
    // calcola # tag in s2 e NON in s1 i.e. tag s2 != tag comune
    int score = 0;
    return score;
}

// INV: v1 and v2 must be vertical photos
int vertical_score(Photo& v1, Photo& v2) {
    int score = v1.tag_num + v2.tag_num;
    for (int i = 0; i < v1.tag_num; i++) {
        for (int j = 0; j < v2.tag_num; j++) {
            if (*(v1.tags[i]) == *(v2.tags[j]))
                score--;
        }
    }
    return score;
}

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

    Slide slides[lines]; // valore finale dopo preproc verticali
    vector<Photo> verticals;

    string kind;
    Photo current_photo;
    int current_slide_id = 0;
    for (int l = 0; l < lines; l++) {
        infile >> kind;
        infile >> current_photo.tag_num;

        current_photo.id = l;
        current_photo.tags = new string*[current_photo.tag_num];
        for (int t = 0; t < current_photo.tag_num; t++) {
            current_photo.tags[t] = new string;
            infile >> *current_photo.tags[t];
        }

        if (kind == "H") {
            slides[current_slide_id].kind = SlideKind::H;
            slides[current_slide_id].horizontal_photo = current_photo; // ptr?
            current_slide_id++;
        } else
            verticals.push_back(current_photo); // ptr?
    }

    // vertical_slides = local_search_verticals(verticals)
    // aggiungi vertical_slides a slides
    // slides = local_search_whole(slides)
    // produci out scorrendo le slides i.e. come sono ordinate e' l'output

    return 0;
}