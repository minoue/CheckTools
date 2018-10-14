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

BentleyOttman::BentleyOttman(std::vector<LineSegment>& edgeVector)
{
    this->edges = edgeVector;
    this->groupId = "";
    this->verbose = false;
}

BentleyOttman::BentleyOttman(std::vector<LineSegment>& edgeVector, std::string& groupId)
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

void BentleyOttman::createNewEvent(LineSegment *lineA, LineSegment *lineB) {
    float x, y;
    lineUtils::getIntersectionPoint(*lineA, *lineB, x, y);
    Point2D p(x, y, 0);
    Event crossEvent(2, lineA, lineB, p, 0);
    eventQueue.insert(crossEvent);
}

void BentleyOttman::check()
{
    int eventIndex = 0;

    std::vector<LineSegment>::iterator edgeIter;

    for (edgeIter = edges.begin(); edgeIter != edges.end(); ++edgeIter) {
        Event ev1(0, &(*edgeIter), (*edgeIter).begin, eventIndex);
        eventIndex++;
        Event ev2(1, &(*edgeIter), (*edgeIter).end, eventIndex);
        eventIndex++;

        eventQueue.insert(ev1);
        eventQueue.insert(ev2);
    }

    statusQueue.reserve(edges.size());
    statusPtrQueue.reserve(edges.size());

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
    LineSegment* currentEdgePtr = ev.edgePtrA;
    statusPtrQueue.emplace_back(currentEdgePtr);

    if (statusPtrQueue.size() == 1) {
        return false;
    }

    // Set crossing point of Y for all edges in the statusQueue and the sweepline
    for (size_t i=0; i<statusPtrQueue.size(); i++) {
        LineSegment* ePtr = statusPtrQueue[i];
        float slope2 = (ePtr->end.y - ePtr->begin.y) / (ePtr->end.x - ePtr->begin.x);
        float b2 = ePtr->begin.y - slope2 * ePtr->begin.x;
        float y2 = slope2 * ev.sweepline + b2;
        ePtr->crossingPointY = y2;
    }
    std::sort(statusPtrQueue.begin(), statusPtrQueue.end(), EdgeCrossingComparator());

    // StatusQueue was sorted so you have to find the edge added to the queue above and find its index
    std::vector<LineSegment*>::iterator foundIter = std::find(statusPtrQueue.begin(), statusPtrQueue.end(), currentEdgePtr);
    if (foundIter == statusPtrQueue.end()) {
        return false;
    }

    // Get currentEdge object from the statusQueue after sorted
    long index = std::distance(statusPtrQueue.begin(), foundIter);

    if (foundIter == statusPtrQueue.begin()) {
        LineSegment* targetEdge = statusPtrQueue[index+1];
        if (*(*foundIter) * *(targetEdge)) {
            resultPtr.emplace_back(*foundIter);
            resultPtr.emplace_back(targetEdge);
            createNewEvent(currentEdgePtr, targetEdge);
        }
    } else if (foundIter == statusPtrQueue.end() - 1) {
        LineSegment* targetEdge = statusPtrQueue[index-1];
        if (*(*foundIter) * *(targetEdge)) {
            resultPtr.emplace_back(*foundIter);
            resultPtr.emplace_back(targetEdge);
            createNewEvent(currentEdgePtr, targetEdge);
        }
    } else {
        LineSegment* nextEdgePtr = statusPtrQueue[index + 1];
        LineSegment* previousEdgePtr = statusPtrQueue[index - 1];
        if (*(*foundIter) * *(nextEdgePtr)) {
            resultPtr.emplace_back(*foundIter);
            resultPtr.emplace_back(nextEdgePtr);
            createNewEvent(currentEdgePtr, nextEdgePtr);
        }
        if (*(*foundIter) * *(previousEdgePtr)) {
            resultPtr.emplace_back(*foundIter);
            resultPtr.emplace_back(previousEdgePtr);
            createNewEvent(currentEdgePtr, previousEdgePtr);
        }
    }
    return true;
}

bool BentleyOttman::doEnd(Event& ev)
{
    LineSegment* currentEdgePtr = ev.edgePtrA;
    std::vector<LineSegment*>::iterator foundIter;
    foundIter = std::find(statusPtrQueue.begin(), statusPtrQueue.end(), currentEdgePtr);

    if (foundIter == statusPtrQueue.end()) {
        // if iter not found
        std::cout << "Not found" << std::endl;
        return false;
    }

    if (foundIter == statusPtrQueue.begin() || foundIter == statusPtrQueue.end() - 1) {

    } else {
        long index = std::distance(statusPtrQueue.begin(), foundIter);
        LineSegment* nextEdgePtr = statusPtrQueue[index + 1];
        LineSegment* previousEdgePtr = statusPtrQueue[index - 1];
        bool isCrossing = (*nextEdgePtr) * (*previousEdgePtr);
        if (isCrossing) {
            resultPtr.emplace_back(nextEdgePtr);
            resultPtr.emplace_back(previousEdgePtr);
            createNewEvent(nextEdgePtr, previousEdgePtr);
        }
    }

    // Remove current edge from the statusQueue
    statusPtrQueue.erase(foundIter);

    return true;
}

bool BentleyOttman::doCross(Event& ev)
{
    if (statusPtrQueue.size() <= 2) {
        return false;
    }

    LineSegment* edgePtr = ev.edgePtrA;
    LineSegment* otherEdgePtr = ev.edgePtrB;

    std::vector<LineSegment*>::iterator lineAPtrIter = std::find(statusPtrQueue.begin(), statusPtrQueue.end(), edgePtr);
    std::vector<LineSegment*>::iterator lineBPtrIter = std::find(statusPtrQueue.begin(), statusPtrQueue.end(), otherEdgePtr);


    if (lineAPtrIter == statusPtrQueue.end() || lineBPtrIter == statusPtrQueue.end()) {
        return false;
    }

    size_t lineAIndex = std::distance(statusPtrQueue.begin(), lineAPtrIter);
    size_t lineBIndex = std::distance(statusPtrQueue.begin(), lineBPtrIter);
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
        if (statusPtrQueue.size() == big + 1) {
            return false;
        }

        LineSegment* lineAPtr = statusPtrQueue[small];
        LineSegment* lineBPtr = statusPtrQueue[big + 1];
        bool isCrossing = (*lineAPtr) * (*lineBPtr);
        if (isCrossing) {
            resultPtr.emplace_back(lineAPtr);
            resultPtr.emplace_back(lineBPtr);
            createNewEvent(lineAPtr, lineBPtr);
        }
    } else if (big == statusPtrQueue.size() - 1) {
        // Check the last edge and the one before the previous edge

        LineSegment* lineAPtr = statusPtrQueue[small - 1];
        LineSegment* lineBPtr = statusPtrQueue[big];
        bool isCrossing = (*lineAPtr) * (*lineBPtr);
        if (isCrossing) {
            resultPtr.emplace_back(lineAPtr);
            resultPtr.emplace_back(lineBPtr);
            createNewEvent(lineAPtr, lineBPtr);
        }
    } else {
        // Check the first edge and the one after next(third)
        LineSegment* lineAPtr = statusPtrQueue[small - 1];
        LineSegment* lineBPtr = statusPtrQueue[big];
        bool isCrossing = (*lineAPtr) * (*lineBPtr);
        if (isCrossing) {
            resultPtr.emplace_back(lineAPtr);
            resultPtr.emplace_back(lineBPtr);
            createNewEvent(lineAPtr, lineBPtr);
        }
        // Check the second edge and the one after next(forth)
        LineSegment* lineCPtr = statusPtrQueue[small];
        LineSegment* lineDPtr = statusPtrQueue[big + 1];
        isCrossing = (*lineCPtr) * (*lineDPtr);
        if (isCrossing) {
            resultPtr.emplace_back(lineCPtr);
            resultPtr.emplace_back(lineDPtr);
            createNewEvent(lineCPtr, lineDPtr);
        }
    }
    return false;
}
