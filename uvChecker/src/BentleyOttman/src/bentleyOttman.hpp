//
//  bentleyOttman.hpp
//  bentleyOttman
//

#ifndef bentleyOttman_hpp
#define bentleyOttman_hpp

#include "event.hpp"
#include "lineSegment.hpp"

#include <set>
#include <string>
#include <vector>

class BentleyOttman {
public:
    BentleyOttman();
    BentleyOttman(std::vector<LineSegment> edgeVector);
    BentleyOttman(std::vector<LineSegment> edgeVector, std::string groupId);
    ~BentleyOttman();

    void check();
    std::vector<LineSegment> result;
    std::vector<LineSegment> edges;
    std::string groupId;

    BentleyOttman operator+(const BentleyOttman& rhs) const;

private:
    bool verbose;
    bool doBegin(Event& ev);
    bool doEnd(Event& ev);
    bool doCross(Event& ev);
    void createNewEvent(LineSegment lineA, LineSegment lineB);
    void assignGroupId();
    std::vector<LineSegment> statusQueue;
    std::multiset<Event> eventQueue;
};

#endif /* bentleyOttman_hpp */
