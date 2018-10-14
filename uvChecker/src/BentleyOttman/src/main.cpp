//
//  main.cpp
//  bentleyOttman
//

#include <algorithm>
#include <iostream>
#include <vector>

#include "bentleyOttman.hpp"
#include "lineSegment.hpp"
#include "point2D.hpp"
#include "testData/dataSet.hpp"


int test() {

    std::vector<LineSegment> edgeVec;
    std::vector<LineSegment> edgeVec2;

    std::string dataPathA = "./testData/DataA-Table_1.csv";
    std::string dataPathB = "./testData/DataB-Table_1.csv";
    std::vector<std::vector<std::string> > v1 = TestDataSet::getDataSet(dataPathA);
    std::vector<std::vector<std::string> > v2 = TestDataSet::getDataSet(dataPathB);

    for (size_t i = 0; i < v1.size(); i++) {
        int indexA = std::stoi(v1[i][0]);
        float x1 = std::stof(v1[i][1]);
        float y1 = std::stof(v1[i][2]);

        int indexB = std::stoi(v1[i][3]);
        float x2 = std::stof(v1[i][4]);
        float y2 = std::stof(v1[i][5]);

        Point2D p1(x1, y1, indexA);
        Point2D p2(x2, y2, indexB);

        LineSegment edge(p1, p2);
        edgeVec.push_back(edge);
    }

    for (size_t i = 0; i < v2.size(); i++) {
        int indexA = std::stoi(v2[i][0]);
        float x1 = std::stof(v2[i][1]);
        float y1 = std::stof(v2[i][2]);

        int indexB = std::stoi(v2[i][3]);
        float x2 = std::stof(v2[i][4]);
        float y2 = std::stof(v2[i][5]);

        Point2D p1(x1, y1, indexA);
        Point2D p2(x2, y2, indexB);

        LineSegment edge(p1, p2);
        edgeVec2.push_back(edge);
    }

    // Remove duplicated line segments
    std::sort(edgeVec.begin(), edgeVec.end(), EdgeIndexComparator());
    edgeVec.erase(std::unique(edgeVec.begin(), edgeVec.end()), edgeVec.end());

    std::sort(edgeVec2.begin(), edgeVec2.end(), EdgeIndexComparator());
    edgeVec.erase(std::unique(edgeVec2.begin(), edgeVec2.end()), edgeVec2.end());

    // Name line group name if necessary
    std::string groupNameA = "lineGroupA";
    std::string groupNameB = "lineGroupB";
    BentleyOttman checker(edgeVec, groupNameA);
    BentleyOttman checker2(edgeVec2, groupNameB);

    BentleyOttman checker3 = checker + checker2;
    checker3.check();

    std::vector<LineSegment*>::iterator resultIter;
    for (resultIter = checker3.resultPtr.begin(); resultIter != checker3.resultPtr.end(); ++resultIter) {
        const LineSegment& line = *(*resultIter);
        std::cout << line.begin.index << ":" << line.end.index << " :: " << std::endl;
        std::cout << line.groupId << std::endl;
    }
    return 0;
}

int main(int argc, const char* argv[])
{
    test();
    std::cout << "end" << std::endl;
    return 0;
}
