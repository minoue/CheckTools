//
//  dataSet.cpp
//  bentleyOttman
//

#include "dataSet.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

std::vector<std::vector<std::string> > TestDataSet::getDataSet(std::string path)
{
    std::ifstream ifs(path.c_str());
    if (!ifs) {
        std::cout << "Failed to open file" << std::endl;
    }
    std::string str;
    std::vector<std::vector<std::string> > v;

    while (std::getline(ifs, str)) {
        std::istringstream stream(str);
        std::string field;
        std::vector<std::string> result;
        while (std::getline(stream, field, ',')) {
            result.push_back(field);
        }
        v.push_back(result);
    }
    return v;
}
