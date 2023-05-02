#include "seamCarving.h"
#include <iostream>

#include "../libs/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../libs/stb_image_write.h"

using namespace std;

SeamCarving::SeamCarving(const std::string& filename) {
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
    for (int i = 0; i < num_seams; ++i) {
        auto energy_map = computeEnergy();
        auto seam = findSeam(energy_map);
        removeSeam(seam);
    }
}

bool SeamCarving::saveEnergyToFile(const std::string& filename) {
	auto energy_map = computeEnergy();

	unsigned char* energy = (unsigned char*) malloc(m_width*m_height*4*sizeof(unsigned char));

	for (int y = 0; y < m_height; ++y)
	for (int x = 0; x < m_width; ++x){
        int idx = (y * m_width + x) * 4;
		energy[idx+0] = energy_map[y][x];
		energy[idx+1] = energy_map[y][x];
		energy[idx+2] = energy_map[y][x];
		energy[idx+3] = m_data[idx+3];
	}

    return stbi_write_png(filename.c_str(), m_width, m_height, 4, energy, m_width * 4);
}

vector<vector<double>> SeamCarving::computeEnergy() const {
    vector<vector<double>> energy(m_height, vector<double>(m_width, 0.0));

    // Compute energy for each pixel
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int idx = (y * m_width + x) * 4;

            double dx = 0.0;
            if(x > 0 && x < m_width-1)
            for (int c = 0; c < 3; ++c) {
                int idx_px_right = idx + 4 + c;
                int idx_px_left = idx - 4 + c;

                int p_diff = (int)(m_data[idx_px_left]) - (int)(m_data[idx_px_right]);
                dx += p_diff * p_diff;
            }

            double dy = 0.0;
            if(y > 0 && y < m_height-1)
            for (int c = 0; c < 3; ++c) {
                int idx_px_up = (idx - (m_width * 4)) + c;
                int idx_px_down = (idx + (m_width * 4)) + c;

                double p_diff = static_cast<int>(m_data[idx_px_up]) - static_cast<int>(m_data[idx_px_down]);
                dy += p_diff * p_diff;
            }

            energy[y][x] = sqrt(dx + dy);
        }
    }

    return energy;
}

vector<vector<int>> SeamCarving::findSeam(const vector<vector<double>>& energy) const {
    vector<vector<double>> dp(m_height, vector<double>(m_width, 0.0));
    vector<vector<int>> dp_idx(m_height, vector<int>(m_width, -1));

    for (int x = 0; x < m_width; ++x) {
        dp[0][x] = energy[0][x];
    }

    for (int y = 1; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int min_idx = x;
            double min_val = dp[y - 1][x] + energy[y][x];

            for (int x_offset : {-1, 1}) {
                int nx = x + x_offset;
                if (nx >= 0 && nx < m_width) {
                    double new_val = dp[y - 1][nx] + energy[y][x];
                    if (new_val < min_val) {
                        min_val = new_val;
                        min_idx = nx;
                    }
                }
            }

            dp[y][x] = min_val;
            dp_idx[y][x] = min_idx;
        }
    }

    double min_path_cost = std::numeric_limits<double>::max();
    int seam_end_x = -1;
    for (int x = 0; x < m_width; ++x) {
    	// cout << dp[m_height - 1][x] << endl;
    	if (dp[m_height - 1][x] < min_path_cost) {
            min_path_cost = dp[m_height - 1][x];
            seam_end_x = x;
        }
    }

    // cout << seam_end_x << endl;

    vector<vector<int>> seam(m_height, vector<int>(2, 0));

    int x = seam_end_x;
    for (int y = m_height - 1; y >= 0; --y) {
        seam[y][0] = y;
        seam[y][1] = x;
        x = dp_idx[y][x];
    }

    // for(auto ss : seam){
    // 	for(int sss : ss)
    // 		cout << sss << " ";
	//     cout << endl;
    // }

    return seam;
}

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
