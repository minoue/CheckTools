//
//  dataSet.hpp
//  bentleyOttman
//

#ifndef dataSet_hpp
#define dataSet_hpp

#include <string>
#include <vector>

class TestDataSet {
public:
    static std::vector<std::vector<std::string> > getDataSet(std::string path);
};

#endif /* dataSet_hpp */
