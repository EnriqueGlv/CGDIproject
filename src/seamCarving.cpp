#include "seamCarving.h"
#include <iostream>

#include "../libs/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../libs/stb_image_write.h"

using namespace std;

SeamCarving::SeamCarving(const std::string& filename) {
    useBackwardSearch = false;
    int channels = 4;
    m_data = stbi_load(filename.c_str(), &m_width, &m_height, NULL, channels);
    if (!m_data) {
        cerr << "Couldn't load file " << filename << endl;
    }
}

const unsigned char* SeamCarving::getCarvedData() const {
    return m_data;
}

int SeamCarving::getCarvedWidth() const {
    return m_width;
}

int SeamCarving::getCarvedHeight() const {
    return m_height;
}

void SeamCarving::carve(int num_seams) {
    computeEnergy();
    computeForwardPaths();
    for (int i = 0; i < num_seams; ++i) {
    	cout << i << endl;
        if (useBackwardSearch) {
            auto seam = findBackwardSeam();
            removeSeam(seam);

        } else {
            auto seam = findForwardSeam();
            removeSeam(seam);
        }
    }
}

bool SeamCarving::saveEnergyToFile(const std::string& filename) {
    if(!pixels.size())
    	computeEnergy();

    size_t dataSize = m_width * m_height * 4;
    std::vector<unsigned char> energy_img(dataSize);

    for (int y = 0; y < m_height; ++y) {
        int x = 0;
        for (auto pix : pixels[y]) {
            int idx = (y * m_width + x) * 4;
            unsigned char energyValue = pix.energy;
            
            energy_img[idx]     = energyValue;
            energy_img[idx + 1] = energyValue;
            energy_img[idx + 2] = energyValue;
            energy_img[idx + 3] = pix.alphaVal;
        	++x;
        }
    }

    return stbi_write_png(filename.c_str(), m_width, m_height, 4, energy_img.data(), m_width * 4);
}

// computes energy for a pixel it and 2 iterators for neighbours pixels up and down
double SeamCarving::getPixelEnergy(pixListIter it, pixListIter up, pixListIter down){

    double dx;
    auto g = it, d = it;
    if(it != pixels[it->y].begin() && ++d != pixels[it->y].end()){
    	g--;
    	double rdiff = g->rVal - d->rVal;
    	double gdiff = g->gVal - d->gVal;
    	double bdiff = g->bVal - d->bVal;
    	dx = rdiff*rdiff + gdiff*gdiff + bdiff*bdiff;
    }

    double dy;
    if(it->y > 0 && it->y < m_height-1){
    	// get the up an down pixels from up and down iterators
    	if(up->x < it->x) up++;
    	if(up->x > it->x) up--;
    	if(down->x < it->x) down++;
    	if(down->x > it->x) down--;
   		// compute dy
    	double rdiff = up->rVal - down->rVal;
    	double gdiff = up->gVal - down->gVal;
    	double bdiff = up->bVal - down->bVal;
    	dy = rdiff*rdiff + gdiff*gdiff + bdiff*bdiff;
    }

    return sqrt(dx + dy);
}

void SeamCarving::computeEnergy(){
    
    // Create pixel matrix
    this->pixels = vector<list<scpixel>>(m_height, list<scpixel>());
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
        	int idx = (y * m_width + x) * 4;
            scpixel pix;
            pix.rVal = m_data[idx    ];
            pix.gVal = m_data[idx + 1];
            pix.bVal = m_data[idx + 2];
            pix.x = x;
            pixels[y].push_back(pix);
        }
    }

    // compute energies using getPixelEnergy
    for (int y = 0; y < m_height; ++y){
    	pixListIter up, down; 
    	if(y > 0) up = pixels[y-1].begin();
    	if(y < m_height - 1) down = pixels[y+1].begin();

    	for(auto it = pixels[y].begin(); it != pixels[y].end(); ++it){
    		it->energy = getPixelEnergy(it, up, down);

    		if(y > 0) ++up;
			if(y < m_height - 1) ++down;    		
    	}
    }
}

// compute pix->dp and pix->next given the pixel just above pix
void SeamCarving::computeDp(int y, pixListIter pix, pixListIter up){
	pix->next = pixListIter(last);
    pix->dp = last->dp + pix->energy;

    if(last != pixels[y-1].begin()){
    	last--;
    	double new_val = last->dp + pix->energy;
    	if(new_val < pix->dp){
    		pix->dp   = new_val;
    		pix->next = pixListIter(last);
    	}
    	last++;
    }

    last++;
    if(last != pixels[y-1].end()){
    	double new_val = last->dp + pix->energy;
    	if(new_val < pix->dp){
    		pix->dp   = new_val;
    		pix->next = pixListIter(last);
    	}
    }
}

void SeamCarving::computeForwardPaths(){

	if(!pixels.size())
    	computeEnergy();	

    // Initialize shortest path cost of first row with energy
	for (auto pix = pixels[0].begin(); pix != pixels[0].end(); ++pix)
        pix->dp = pix->energy;

    // Iterate over y to find for each x the best next path
    for (int y = 1; y < m_height; ++y) {
    	pixListIter last = pixels[y-1].begin();

    	for (auto pix = pixels[y].begin(); pix != pixels[y].end(); ++pix){
            computeDp(y, pix, pixListIter(last));
    		++last;
        }
    }
}

vector<pixListIter> SeamCarving::findForwardSeam(){
    
    // if(!pixels.size())
    // 	computeForwardPaths();

    // find seam in O(w) -> because no need to recompute paths
    double min_path_cost = std::numeric_limits<double>::max();
    auto seam_end = pixels[m_height-1].begin();
    for (auto it = pixels[m_height-1].begin(); it != pixels[m_height-1].end(); ++it) {
    	if (it->dp < min_path_cost) {
    		// cout << it->x << " " << it->dp << endl;
            min_path_cost = it->dp;
            seam_end = it;
        }
    }

    // for (auto itx = pixels[m_height-1].begin(); itx != pixels[m_height-1].end(); ++itx){
    // 	pixListIter it = itx;
    // 	cout << "min cost: " << itx->dp << endl; 
    // 	for (int y = m_height - 1; y >= 0; --y) {
	//         cout << it->x << " ";
	//         it = it->next;
	//     }
	//     cout << endl;
    // }

    vector<pixListIter> seam(m_height);

    auto pix = seam_end;
    for (int y = m_height - 1; y >= 0; --y) {
        seam[y] =  pix;
        // cout << pix->x << " ";
        // cout << y << " ";
        // cout << pix->energy << " ";
        // cout << pix->dp << endl;
        pix = pix->next;
    }

    return seam;
}

vector<vector<int>> SeamCarving::findBackwardSeam() const {
    cerr << "Not implemented" << endl;
}

void SeamCarving::removeSeam(const std::vector<pixListIter>& seam){

    // remove seam in O(h)
    for (int y = 0; y < m_height; ++y){
    	// keeps the right neighbor of the removed seam in seam vector
    	seam[y] = pixels[y].erase(seam[y]);
    }

    // update pixels neighbours to the removed seam
    for (int y = 0; y < m_height; ++y) {

    	auto pix = seam[y];

    	// update energy using getPixelEnergy();
    	pixListIter up, down;
    	if(y > 0) up = seam[y-1];
    	if(y < m_height-1) down = seam[y+1];

    	if(pix != pixels[y].end()){
    		if(up == pixels[y-1].end()) up--;
    		if(down == pixels[y+1].end()) down--;
	    	pix->energy = getPixelEnergy(pix, up, down);
    	}

		if(pix != pixels[y].begin()){
			if(y > 0 && up != pixels[y-1].begin()) --up;
    		if(y < m_height-1 && down != pixels[y+1].begin()) --down;
    		--pix;
    		pix->energy = getPixelEnergy(pix, up, down);    	
		}

    	// update dp values
    	if(y > 0){
			if(up == pixels[y-1].end()) --up;

			if(pix != pixels[y].end()){
				if(pix->x < up->pix);
				computeDp(y, pix, up);
			}

			if(pix != pixels[y].begin()){
				--pix;
				computeDp(y, pix, up);
			}
    	}
        
    }

    // should be executed only in graphical mode
    if(1){
	    // re-generate image (slow O(hw) because of data structure)
		int new_width = m_width - 1;
    	int new_size = new_width * m_height * 4;
    	unsigned char* new_data = new unsigned char[new_size];

    	// TODO: clean way using 'pixels' array
    	for (int y = 0; y < m_height; ++y){
    		int seam_x = seam[y]->x;

	        for (int x = 0; x < new_width; ++x) {
	            int new_idx = (y * new_width + x) * 4;
	            int old_idx = (y * m_width + x) * 4;

	            if (x >= seam_x)
	                old_idx += 4;

	            for (int c = 0; c < 4; ++c)
	                new_data[new_idx + c] = m_data[old_idx + c];
	        }
    	}
    	
    	stbi_image_free(m_data);
    	m_data = new_data;
    	m_width -= 1;
    }
}

// old
void SeamCarving::removeSeam(const std::vector<std::vector<int>>& seam) {
    int new_width = m_width - 1;
    int new_size = new_width * m_height * 4;
    unsigned char* new_data = new unsigned char[new_size];

    for (int y = 0; y < m_height; ++y) {
        int seam_x = seam[y][1];
        for (int x = 0; x < new_width; ++x) {
            int new_idx = (y * new_width + x) * 4;
            int old_idx = (y * m_width + x) * 4;

            if (x >= seam_x) {
                old_idx += 4;
            }

            for (int c = 0; c < 4; ++c) {
                new_data[new_idx + c] = m_data[old_idx + c];
            }
        }
    }

    stbi_image_free(m_data);
    m_data = new_data;
    m_width -= 1;
}

bool SeamCarving::saveCarvedImageToFile(const std::string& filename) const {
    return stbi_write_png(filename.c_str(), m_width, m_height, 4, m_data, m_width * 4);
}


/*
LEFT TO DO:

- energy update /!\ don't use pix->x !!! (in getPixelEnergy) 
it becomes irrelevant after 2 iterations of the algo
	-> memorize the location of next seam pixel (left, same x, right) instead

- dp update /!\ same thing !!!

- clean image saving

- re-implement non-optimized algorithm (with a button to choose optim/non-optimized algo)
	(hope we see the diff)

- take time measures to compare perfs (exclude image loading parts from measure)

- report
*/