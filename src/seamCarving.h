#ifndef SEAMCARVING_H
#define SEAMCARVING_H

#include <vector>
#include <list>
#include <string>
#include <cmath>
#include <limits>
#include <algorithm>

typedef struct scpixel scpixel;
typedef std::list<scpixel>::iterator pixListIter;

struct scpixel{
    // png format
    double rVal;
    double gVal;
    double bVal;
    double alphaVal;
    // seam carving utilitaries
    double energy;
    double dp;
    int x;
    pixListIter next;
};


class SeamCarving {
public:
    SeamCarving(const std::string& filename);
    // Run seam carving for the desired number of seams.
    void carve(int num_seams);

    const unsigned char* getCarvedData() const;
    int getCarvedWidth() const;
    int getCarvedHeight() const;

    bool useBackwardSearch;

    bool saveEnergyToFile(const std::string& filename);
    bool saveCarvedImageToFile(const std::string& filename) const;

private:
    unsigned char* m_data;
    int m_width;
    int m_height;

    std::vector<std::list<scpixel>> pixels;

    // Helper functions for seam carving algorithm
    double getPixelEnergy(int x, int y);
    void computeEnergy();
    void computeForwardPaths();
    std::vector<pixListIter> findForwardSeam();
    void removeSeam(const std::vector<pixListIter>& seam);
    std::vector<std::vector<int>> findBackwardSeam() const;
    void removeSeam(const std::vector<std::vector<int>>& seam);
};

#endif // SEAMCARVING_H
