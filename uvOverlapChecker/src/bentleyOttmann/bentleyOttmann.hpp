//
//  bentleyOttmann.hpp
//  bentleyOttmann
//

#ifndef bentleyOttmann_hpp
#define bentleyOttmann_hpp

#include "event.hpp"
#include "lineSegment.hpp"

#include <set>
#include <string>
#include <vector>

class BentleyOttmann {
public:
    BentleyOttmann();
    explicit BentleyOttmann(std::vector<LineSegment>& edgeVector);
    ~BentleyOttmann();

    void check(std::vector<LineSegment> &result);
    std::vector<LineSegment> edges;

    BentleyOttmann operator+(const BentleyOttmann& rhs) const;

private:
    std::vector<LineSegment> *resultPtr;
    bool doBegin(Event& ev);
    bool doEnd(Event& ev);
    bool doCross(Event& ev);
    void createNewEvent(LineSegment* lineA, LineSegment* lineB);

    std::vector<LineSegment*> statusPtrQueue;
    std::multiset<Event> eventQueue;
};

#endif /* bentleyOttmann_hpp */
