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

class BentleyOttmann {
public:
    BentleyOttmann();
    explicit BentleyOttmann(std::vector<LineSegment>& edgeVector);
    BentleyOttmann(std::vector<LineSegment>& edgeVector, std::string& groupId);
    ~BentleyOttmann();

    void check();
    std::vector<LineSegment> result;
    std::vector<LineSegment*> resultPtr;
    std::vector<LineSegment> edges;
    std::string groupId;

    BentleyOttmann operator+(const BentleyOttmann& rhs) const;

private:
    bool verbose;
    bool doBegin(Event& ev);
    bool doEnd(Event& ev);
    bool doCross(Event& ev);
    void createNewEvent(LineSegment* lineA, LineSegment* lineB);
    void assignGroupId();

    std::vector<LineSegment*> statusPtrQueue;
    std::multiset<Event> eventQueue;
};

#endif /* bentleyOttman_hpp */
