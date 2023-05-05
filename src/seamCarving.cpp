#include "seamCarving.h"
#include <iostream>

#include "../libs/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../libs/stb_image_write.h"

using namespace std;

// Constructor
SeamCarving::SeamCarving(const string& filename, Settings s) {
    settings = s;
    int channels = 4;

    // Load image data
    auto data = stbi_load(filename.c_str(), &m_width, &m_height, NULL, channels);
    if (!data) {
        cerr << "Couldn't load file " << filename << endl;
    }

    // Convert loaded image data into list of Pixel for further processing
    m_data = vector(m_height, list<Pixel>());
    for (int x = 0; x < m_height; x++) {
        for (int y = 0; y < m_width; y++) {
            Pixel p;
            int idx = (m_width * x + y) * 4;
            p.r = data[idx];
            p.g = data[idx + 1];
            p.b = data[idx + 2];
            p.a = data[idx + 3];
            m_data[x].push_back(p);
        }
    }
}

// Getter functions
vector<list<Pixel>> SeamCarving::getCarvedData() const {
    return m_data;
}

int SeamCarving::getCarvedWidth() const {
    return m_width;
}

int SeamCarving::getCarvedHeight() const {
    return m_height;
}

// Main carve function
void SeamCarving::carve(int num_seams) {
    computeEnergy();
    for (int i = 0; i < num_seams; ++i) {
        if (settings.doBackwardSearch) {
            auto seam = findBackwardSeam();
            removeSeam(seam);

        } else {
            auto seam = findForwardSeam();
            removeSeam(seam);
        }
    }
}

// Save the computed energy values into an image file
bool SeamCarving::saveEnergyToFile(const string& filename) {
    size_t dataSize = m_width * m_height * 4;
    vector<unsigned char> energy_img(dataSize);

    // Convert energy values into a linear unsigned char array
    for (int x = 0; x < m_height; ++x) {
        auto ite = energy[x].begin();
        auto itd = m_data[x].begin();
        for (int y = 0; y < m_width; y++) {
            int idx = (x * m_width + y) * 4;

            energy_img[idx]     = *ite;
            energy_img[idx + 1] = *ite;
            energy_img[idx + 2] = *ite;
            energy_img[idx + 3] = (*itd).a;
            ite = next(ite);
            itd = next(itd);
        }
    }

    // Save the image using STB Image library
    return stbi_write_png(filename.c_str(), m_width, m_height, 4, energy_img.data(), m_width * 4);
}

double pixel_diff_squared(const Pixel &a, const Pixel &b) {
    double dr = static_cast<int>(a.r) - static_cast<int>(b.r);
    double dg = static_cast<int>(a.g) - static_cast<int>(b.g);
    double db = static_cast<int>(a.b) - static_cast<int>(b.b);
    
    return dr * dr + dg * dg + db * db;
}

Pixel SeamCarving::getPixel(int y, int x) const {
    auto it = m_data[y].begin();
    std::advance(it, x);
    return *it;
}

// Compute energy for each pixel based on the color gradient
void SeamCarving::computeEnergy() {
    energy = vector<list<double>>(m_height, list<double>());

    // Compute energy for each pixel
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            auto it = m_data[y].begin();
            advance(it, x);
            Pixel px = *it;

            double dx = 0.0;
            if (x > 0 && x < m_width - 1) {
                dx = pixel_diff_squared(*next(it), *prev(it));
            }

            double dy = 0.0;
            if (y > 0 && y < m_height - 1) {
                dy += pixel_diff_squared(getPixel(y+1, x), getPixel(y-1, x));
            }
            energy[y].push_back(sqrt(dx + dy));
        }
    }
}

vector<vector<int>> SeamCarving::findBackwardSeam() const {
    // Implementation of the findBackwardSeam function
    // The dynamic programming table (dp) stores the lowest energy cost to reach each pixel
    // It also keeps track of the path that led to this lowest cost (dp_idx)
    vector<vector<double>> dp(m_height, vector<double>(m_width, 0.0));
    vector<vector<int>> dp_idx(m_height, vector<int>(m_width, -1));

    int x = 0;
    for (auto v : energy[0]) {
        dp[0][x] = v;
        x++;
    }

    for (int y = 1; y < m_height; ++y) {
        x = 0;
        for (auto v : energy[y]) {
            int min_idx = x;
            double min_val = dp[y - 1][x] + v;

            for (int x_offset : {-1, 1}) {
                int nx = x + x_offset;
                if (nx >= 0 && nx < m_width) {
                    double new_val = dp[y - 1][nx] + v;
                    if (new_val < min_val) {
                        min_val = new_val;
                        min_idx = nx;
                    }
                }
            }

            dp[y][x] = min_val;
            dp_idx[y][x] = min_idx;
            x++;
        }
    }

    double min_path_cost = std::numeric_limits<double>::max();
    int seam_end_x = -1;
    for (int x = 0; x < m_width; ++x) {
    	if (dp[m_height - 1][x] < min_path_cost) {
            min_path_cost = dp[m_height - 1][x];
            seam_end_x = x;
        }
    }

    vector<vector<int>> seam(m_height, vector<int>(2, 0));

    x = seam_end_x;
    for (int y = m_height - 1; y >= 0; --y) {
        seam[y][0] = y;
        seam[y][1] = x;
        x = dp_idx[y][x];
    }

    return seam;
}

vector<vector<int>> SeamCarving::findForwardSeam() const {
    // Implementation of the findForwardSeam function
    // The difference between forward and backward seam carving methods is that the forward method accounts
    // for the energy introduced by connecting neighboring pixels when a seam has been removed,
    // whereas the backward method ignores this newly introduced energy.
    // The forward method often yields better visual results.

    // The dynamic programming table (dp) stores the lowest energy cost to reach each pixel
    // It also keeps track of the path that led to this lowest cost (dp_idx)
    vector<vector<double>> dp(m_height, vector<double>(m_width, std::numeric_limits<double>::max()));
    vector<vector<int>> dp_idx(m_height, vector<int>(m_width, -1));

    for (int x = 0; x < m_width; ++x) {
        dp[0][x] = 0.0;
    }

    for (int y = 0; y < m_height - 1; ++y) {
        for (int x = 0; x < m_width; ++x) {
            double left_energy = x > 0 ? pixel_diff_squared(getPixel(y + 1, x - 1), getPixel(y, x + 1)) : std::numeric_limits<double>::max();
            double right_energy = x < m_width - 1 ? pixel_diff_squared(getPixel(y + 1, x + 1), getPixel(y, x - 1)) : std::numeric_limits<double>::max();

            auto current_energy = energy[y + 1].begin();
            std::advance(current_energy, x);

            if (dp[y][x] + left_energy < dp[y + 1][x - 1]) {
                dp[y + 1][x - 1] = dp[y][x] + left_energy;
                dp_idx[y + 1][x - 1] = x;
            }

            if (dp[y][x] + (*current_energy) < dp[y + 1][x]) {
                dp[y + 1][x] = dp[y][x] + (*current_energy);
                dp_idx[y + 1][x] = x;
            }

            if (dp[y][x] + right_energy < dp[y + 1][x + 1]) {
                dp[y + 1][x + 1] = dp[y][x] + right_energy;
                dp_idx[y + 1][x + 1] = x;
            }
        }
    }

    double min_path_cost = std::numeric_limits<double>::max();
    int seam_end_x = -1;
    for (int x = 0; x < m_width; ++x) {
        if (dp[m_height - 1][x] < min_path_cost) {
            min_path_cost = dp[m_height - 1][x];
            seam_end_x = x;
        }
    }

    vector<vector<int>> seam(m_height, vector<int>(2, 0));

    int x = seam_end_x;
    for (int y = m_height - 1; y >= 0; --y) {
        seam[y][0] = y;
        seam[y][1] = x;
        x = dp_idx[y][x];
    }

    return seam;
}

// This function takes the computed seam and removes it from the original image.
void SeamCarving::removeSeam(const std::vector<std::vector<int>>& seam) {
    // Compute new width of the image after removal of the seam
    int new_width = m_width - 1;

    // Iterate through each row of the image and remove the pixel that belongs to the seam from m_data and energy tables
    for (int y = 0; y < m_height; ++y) {
        int seam_x = seam[y][1];
        
        std::list<double>::iterator energy_it = energy[y].begin();
        std::advance(energy_it, seam_x);
        energy[y].erase(energy_it);

        std::list<Pixel>::iterator data_it = m_data[y].begin();
        std::advance(data_it, seam_x);
        m_data[y].erase(data_it);
    }
    
    // Update the width of the image
    m_width = new_width;
}

bool SeamCarving::saveCarvedImageToFile(const std::string& filename) const {
    int num_channels = 4;

    // Allocate memory for the linear unsigned char array
    unsigned char* linear_data = new unsigned char[m_width * m_height * num_channels];

    // Convert the m_data structure to a linear unsigned char array
    for (int x = 0; x < m_height; ++x) {
        int y = 0;
        for (const Pixel& pixel : m_data[x]) {
            int idx = (x * m_width + y) * num_channels;
            linear_data[idx] = pixel.r;
            linear_data[idx + 1] = pixel.g;
            linear_data[idx + 2] = pixel.b;
            linear_data[idx + 3] = pixel.a;
            ++y;
        }
    }

    // Save the image using stbi_write_png()
    bool success = stbi_write_png(filename.c_str(), m_width, m_height, num_channels, linear_data, m_width * num_channels);

    // Free the memory allocated for the linear unsigned char array
    delete[] linear_data;

    return success;
}
