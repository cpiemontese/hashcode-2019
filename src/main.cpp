#include <tuple>
#include <string>
#include <vector>
#include <random>
#include <math.h>
#include <fstream>
#include <iostream>
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
    vector<Photo> photos(lines);
    for (int l = 0; l < lines; l++) {
        infile >> photos[l].kind;
        infile >> photos[l].tag_num;

        if (photos[l].kind == "V")
            vertical_photos++;

        photos[l].id = l;
        for (int t = 0; t < photos[l].tag_num; t++) {
            string tag;
            infile >> tag;
            photos[l].tags.push_back(tag);
        }
    }

    // TODO: deal with the case when there are no vertical/horizontal slides
    int v = 0;
    int vslides_num = floor(vertical_photos/2);
    vector<Photo*> vphotos(vertical_photos);
    for (int l = 0; l < lines && v < vertical_photos; l++) {
        if (photos[l].kind == "V") {
            vphotos[v] = &photos[l];
            v++;
        }
    }

    vector<Slide> vertical_slides(vertical_photos);
    //local_search_verticals(vphotos, vertical_photos, vslides_num, vertical_slides);
    local_search<Photo*, Slide>(vphotos, vertical_photos, vertical_slides);

    int slides_num = lines - vertical_photos + vslides_num;
    // prepare slides
    vector<Slide> slides(slides_num);
    int slide_id = 0;
    for (int i = 0; i < lines; i++) {
        if (!(photos[i].kind == "H")) continue;
        copy_photo_to_hslide(photos[i], slides[slide_id]);
        slide_id++;
    }
    for (int j = 0; j < vslides_num; j++) {
        copy(vertical_slides[j], slides[slide_id]);
        slide_id++;
    }

    //local_search_slides(slides, slides_num, slides);
    local_search<Slide, Slide>(slides, slides_num, slides);

    return 0;
}
