#ifndef SEAMCARVING_H
#define SEAMCARVING_H

#include <vector>
#include <string>
#include <cmath>
#include <limits>
#include <algorithm>

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
    //std::vector<unsigned char> m_data;
    unsigned char* m_data;
    int m_width;
    int m_height;

    std::vector<std::vector<double>> energy;

    // Helper functions for seam carving algorithm
    void computeEnergy();
    std::vector<std::vector<int>> findForwardSeam() const;
    std::vector<std::vector<int>> findBackwardSeam() const;
    void removeSeam(const std::vector<std::vector<int>>& seam);
};

#endif // SEAMCARVING_H
