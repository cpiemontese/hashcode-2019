#include <fstream>
#include <iostream>
#include <math.h>
#include <random>
#include <tuple>
#include <photo.h>
#include <slide.h>
#include <local_search.h>
using namespace std;

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
