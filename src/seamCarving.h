#ifndef SEAMCARVING_H
#define SEAMCARVING_H

#include <vector>
#include <string>
#include <cmath>
#include <list>
#include <limits>
#include <algorithm>

#include "settings.h"

using namespace std;

struct Pixel {
    unsigned char r, g, b, a;
};

class SeamCarving {
public:
    SeamCarving(const std::string& filename, Settings s);
    // Run seam carving for the desired number of seams.
    void carve(int num_seams);

    vector<list<Pixel>> getCarvedData() const;
    int getCarvedWidth() const;
    int getCarvedHeight() const;

    Settings settings;

    Pixel getPixel(int x, int y) const;

    bool saveEnergyToFile(const std::string& filename);
    bool saveCarvedImageToFile(const std::string& filename) const;

private:
    vector<list<Pixel>> m_data;
    //unsigned char* m_data;
    int m_width;
    int m_height;

    vector<list<double>> energy;

    // Helper functions for seam carving algorithm
    void computeEnergy();
    vector<vector<int>> findForwardSeam() const;
    vector<vector<int>> findBackwardSeam() const;
    void removeSeam(const vector<vector<int>>& seam);
};

#endif // SEAMCARVING_H
