#ifndef ZHURIN_I_EDGE_DETECTION_COMMON_HPP
#define ZHURIN_I_EDGE_DETECTION_COMMON_HPP

#include <fstream>
#include <string>
#include <tuple>
#include <vector>
#include <stdexcept>
#include "task/include/task.hpp"

namespace zhurin_i_sobel_edge {

enum ImageSet {
    SAMPLE_1 = 0,
    SAMPLE_2,
    SAMPLE_3, 
    SAMPLE_4,
    SAMPLE_5,
    SAMPLE_6,
    SAMPLE_7
};

using ImageTuple = std::tuple<std::vector<int>, int, int, int>;
using ResultVector = std::vector<int>;
using TestPair = std::tuple<ImageSet, std::string>;
using TaskInterface = ppc::task::Task<ImageTuple, ResultVector>;

inline std::tuple<std::vector<int>, int, int> importImage(const std::string& path) {
    std::ifstream stream(path);
    if (!stream) {
        throw std::runtime_error("File not found: " + path);
    }
    
    int h, w;
    stream >> h >> w;
    
    std::vector<int> matrix(h * w);
    for (int idx = 0; idx < h * w; ++idx) {
        if (!(stream >> matrix[idx])) {
            matrix.clear();
            h = w = 0;
            break;
        }
    }
    
    return {matrix, h, w};
}

inline std::string extractRootDir(const std::string& configPath) {
    const std::string suffix = "settings.json";
    std::string dir = configPath;
    
    auto pos = dir.find(suffix);
    if (pos != std::string::npos) {
        dir.erase(pos);
    }
    
    return dir;
}

inline std::tuple<std::vector<int>, int, int, int> prepareInputData(ImageSet sample) {
    std::string filename;
    switch(sample) {
        case SAMPLE_1: filename = "img1.txt"; break;
        case SAMPLE_2: filename = "img2.txt"; break;
        case SAMPLE_3: filename = "img3.txt"; break;
        case SAMPLE_4: filename = "img4.txt"; break;
        case SAMPLE_5: filename = "img5.txt"; break;
        case SAMPLE_6: filename = "img6.txt"; break;
        case SAMPLE_7: filename = "img7.txt"; break;
        default: filename = "img1.txt";
    }
    
    std::string base_path = extractRootDir(PPC_SETTINGS_zhurin_i_edge_sobel);
    std::string full_path = base_path + "data/cases/" + filename;
    
    auto [pixels, height, width] = importImage(full_path);
    int threshold = 100;
    
    return {pixels, height, width, threshold};
}

inline ResultVector fetchExpected(ImageSet sample) {
    std::string filename;
    switch(sample) {
        case SAMPLE_1: filename = "img1.txt"; break;
        case SAMPLE_2: filename = "img2.txt"; break;
        case SAMPLE_3: filename = "img3.txt"; break;
        case SAMPLE_4: filename = "img4.txt"; break;
        case SAMPLE_5: filename = "img5.txt"; break;
        case SAMPLE_6: filename = "img6.txt"; break;
        case SAMPLE_7: filename = "img7.txt"; break;
        default: filename = "img1.txt";
    }
    
    std::string base_path = extractRootDir(PPC_SETTINGS_zhurin_i_edge_sobel);
    std::string full_path = base_path + "data/expected/" + filename;
    
    auto [pixels, height, width] = importImage(full_path);
    
    return pixels;
}

} // namespace zhurin_i_sobel_edge

#endif