//
//  bentleyOttman.cpp
//  bentleyOttman
//

#include "bentleyOttman.hpp"
#include <algorithm>
#include <iostream>

BentleyOttman::BentleyOttman()
{
}

BentleyOttman::BentleyOttman(std::vector<LineSegment> edgeVector)
{
    this->edges = edgeVector;
    this->groupId = "";
    this->verbose = false;
}

BentleyOttman::BentleyOttman(std::vector<LineSegment> edgeVector, std::string groupId)
{
    this->edges = edgeVector;
    this->groupId = groupId;
    this->verbose = false;

    assignGroupId();
}

BentleyOttman::~BentleyOttman()
{
}

BentleyOttman BentleyOttman::operator+(const BentleyOttman& rhs) const
{
    // Create new BentleyOttman object
    size_t newSize = this->edges.size() + rhs.edges.size();
    std::vector<LineSegment> AB;
    AB.reserve(newSize);
    AB.insert(AB.end(), this->edges.begin(), this->edges.end());
    AB.insert(AB.end(), rhs.edges.begin(), rhs.edges.end());

    BentleyOttman newBO(AB);

    return newBO;
}

void BentleyOttman::assignGroupId()
{
    std::vector<LineSegment>::iterator iter;
    for (iter = edges.begin(); iter != edges.end(); ++iter) {
        (*iter).groupId = this->groupId;
    }
}

void BentleyOttman::createNewEvent(LineSegment lineA, LineSegment lineB)
{
    float x, y;
    lineUtils::getIntersectionPoint(lineA, lineB, x, y);
    Point2D p(x, y, 0);
    Event crossEvent(2, lineA, lineB, p, 0);
    eventQueue.insert(crossEvent);
}

void BentleyOttman::check()
{
    int eventIndex = 0;

    std::vector<LineSegment>::iterator edgeIter;

    for (edgeIter = edges.begin(); edgeIter != edges.end(); ++edgeIter) {
        LineSegment& edge = *edgeIter;
        Event ev1(0, *edgeIter, edge.begin, eventIndex);
        eventIndex++;
        Event ev2(1, *edgeIter, edge.end, eventIndex);
        eventIndex++;

        eventQueue.insert(ev1);
        eventQueue.insert(ev2);
    }

    statusQueue.reserve(edges.size());

    while (true) {
        if (eventQueue.empty()) {
            break;
        }

        Event firstEvent = *eventQueue.begin();
        eventQueue.erase(eventQueue.begin());

        switch (firstEvent.eventType) {
        case Event::BEGIN:
            doBegin(firstEvent);
            break;
        case Event::END:
            doEnd(firstEvent);
            break;
        case Event::CROSS:
            doCross(firstEvent);
            break;
        default:
            break;
        }
    }
}

bool BentleyOttman::doBegin(Event& ev)
{
    LineSegment& currentEdge = ev.edge;
    statusQueue.push_back(currentEdge);

    if (statusQueue.size() == 1) {
        // If only one edge is in the statusQueue, just continue...
        return false;
    }

    // Set crossing point of Y for all edges in the statusQueue and the sweepline
    for (size_t i = 0; i < statusQueue.size(); i++) {
        LineSegment& e = statusQueue[i];
        float slope = (e.end.y - e.begin.y) / (e.end.x - e.begin.x);
        // y = ax + b
        float b = e.begin.y - slope * e.begin.x;
        float y = slope * ev.sweepline + b;
        e.crossingPointY = y;
    }
    std::sort(statusQueue.begin(), statusQueue.end(), EdgeCrossingComparator());

    // StatusQueue was sorted so you have to find the edge added to the queue above and find its index
    std::vector<LineSegment>::iterator foundIter;
    foundIter = std::find(statusQueue.begin(), statusQueue.end(), currentEdge);
    if (foundIter == statusQueue.end()) {
        // If the edge was not found in the queue, skip this function and go to next event
        return false;
    }

    // Get currentEdge object from the statusQueue after sorted
    size_t index = std::distance(statusQueue.begin(), foundIter);

    if (foundIter == statusQueue.begin()) {
        // If first item, check with next edge

        LineSegment& line2 = statusQueue[index + 1];
        if (*foundIter * line2) {
            result.push_back(*foundIter);
            result.push_back(line2);
            createNewEvent(currentEdge, line2);
        }

    } else if (foundIter == statusQueue.end() - 1) {
        // if last iten in the statusQueue

        LineSegment& line2 = statusQueue[index - 1];
        if (*foundIter * line2) {
            result.push_back(*foundIter);
            result.push_back(line2);
            createNewEvent(currentEdge, line2);
        }
    } else {
        LineSegment& nextEdge = statusQueue[index + 1];
        LineSegment& previousEdge = statusQueue[index - 1];
        if (*foundIter * nextEdge) {
            result.push_back(*foundIter);
            result.push_back(nextEdge);
            createNewEvent(currentEdge, nextEdge);
        }
        if (*foundIter * previousEdge) {
            result.push_back(*foundIter);
            result.push_back(previousEdge);
            createNewEvent(currentEdge, previousEdge);
        }
    }
    return true;
}

bool BentleyOttman::doEnd(Event& ev)
{
    LineSegment& currentEdge = ev.edge;
    std::vector<LineSegment>::iterator foundIter;
    foundIter = std::find(statusQueue.begin(), statusQueue.end(), currentEdge);

    if (foundIter == statusQueue.end()) {
        // if iter not found
        std::cout << "Not found" << std::endl;
        return false;
    }

    if (foundIter == statusQueue.begin() || foundIter == statusQueue.end() - 1) {
        // if first or last item, do nothing
    } else {
        // check previous and next edge intersection as they can be next
        // each other after removing the current edge

        size_t index = std::distance(statusQueue.begin(), foundIter);

        LineSegment& nextEdge = statusQueue[index + 1];
        LineSegment& previousEdge = statusQueue[index - 1];

        bool isCrossing = nextEdge * previousEdge;
        if (isCrossing) {

            result.push_back(nextEdge);
            result.push_back(previousEdge);
            createNewEvent(nextEdge, previousEdge);
        }
    }

    // Remove current edge from the statusQueue
    statusQueue.erase(foundIter);

    return true;
}

bool BentleyOttman::doCross(Event& ev)
{
    if (statusQueue.size() <= 2) {
        return false;
    }

    LineSegment& edge = ev.edge;
    LineSegment& otherEdge = ev.otherEdge;

    std::vector<LineSegment>::iterator lineAIter = std::find(statusQueue.begin(), statusQueue.end(), edge);
    std::vector<LineSegment>::iterator lineBIter = std::find(statusQueue.begin(), statusQueue.end(), otherEdge);

    if (lineAIter == statusQueue.end() || lineBIter == statusQueue.end()) {
        return false;
    }

    size_t lineAIndex = std::distance(statusQueue.begin(), lineAIter);
    size_t lineBIndex = std::distance(statusQueue.begin(), lineBIter);
    size_t small, big;

    if (lineAIndex > lineBIndex) {
        small = lineBIndex;
        big = lineAIndex;
    } else {
        small = lineAIndex;
        big = lineBIndex;
    }

    if (small == 0) {
        // Check the first edge and the one after next edge

        // If If the second edge is the last element of the statusQueue, then there is
        // no edge to be checked with the first edge
        if (statusQueue.size() == big + 1) {
            return false;
        }

        LineSegment& lineA = statusQueue[small];
        LineSegment& lineB = statusQueue[big + 1];
        bool isCrossing = lineA * lineB;
        if (isCrossing) {
            result.push_back(lineA);
            result.push_back(lineB);
            createNewEvent(lineA, lineB);
        }

    } else if (big == statusQueue.size() - 1) {
        // Check the last edge and the one before the previous edge

        LineSegment& lineA = statusQueue[small - 1];
        LineSegment& lineB = statusQueue[big];
        bool isCrossing = lineA * lineB;
        if (isCrossing) {
            result.push_back(lineA);
            result.push_back(lineB);
            createNewEvent(lineA, lineB);
        }
    } else {
        // Check the first edge and the one after next(third)
        LineSegment& lineA = statusQueue[small - 1];
        LineSegment& lineB = statusQueue[big];
        bool isCrossing = lineA * lineB;
        if (isCrossing) {
            result.push_back(lineA);
            result.push_back(lineB);
            createNewEvent(lineA, lineB);
        }

        // Check the second edge and the one after next(forth)
        LineSegment& lineC = statusQueue[small];
        LineSegment& lineD = statusQueue[big + 1];
        isCrossing = lineC * lineD;
        if (isCrossing) {
            result.push_back(lineC);
            result.push_back(lineD);
            createNewEvent(lineA, lineB);
        }
    }

    return false;
}
