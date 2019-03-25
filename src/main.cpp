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
    cout << "total vertical: " << vertical_photos << ", slides: " << vslides_num << endl;
    Photo* vphotos[vertical_photos];
    for (int l = 0; l < lines && v < vertical_photos; l++) {
        if (photos[l].kind == "V") {
            vphotos[v] = &photos[l];
            v++;
        }
    }

    Slide vertical_slides[vslides_num];
    local_search_verticals(vphotos, vertical_photos, vslides_num, vertical_slides);

    int slides_num = lines - vertical_photos + vslides_num;
    // prepare slides
    Slide slides[slides_num];
    for (int i = 0; i < lines; i++) {
        if (!(photos[i].kind == "H")) continue;
        copy_photo_to_hslide(photos[i], slides[i]);
    }
    for (int i = lines, j = 0; i < slides_num & j < vslides_num; i++, j++)
        copy(vertical_slides[j], slides[i]);

    local_search_slides(slides, slides_num, slides);

    return 0;
}
