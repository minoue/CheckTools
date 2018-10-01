#include "aabb.h"
#include "iostream"
#include <algorithm>


AABB::Point::Point()
{
}

AABB::Point::Point(float x, float y)
{
    this->x = x;
    this->y = y;
}

AABB::Point::~Point()
{
}

AABB::Triangle::Triangle()
{
}

AABB::Triangle::Triangle(Point a, Point b, Point c, std::string fullPath)
{
    this->A = a;
    this->B = b;
    this->C = c;

    float mx = (a.x + b.x + c.x) / 3.0;
    float my = (a.y + b.y + c.y) / 3.0;

    this->medianPoint.x = mx;
    this->medianPoint.y = my;

    this->fullPath = fullPath;
}

AABB::Triangle::~Triangle()
{
}

AABB::Node::Node()
{
}

AABB::Node::~Node()
{
}

bool AABB::Node::operator*(const Node &rhs) const
{
    if (this->xMax < rhs.xMin)
        return false;
    
    if (this->xMin > rhs.xMax)
        return false;
    
    if (this->yMax < rhs.yMin)
        return false;
    
    if (this->yMin > rhs.yMax)
        return false;
    
    return true;    
}

void AABB::Node::printAABB() {
    std::cout << "--------" << std::endl;
    std::cout << "xMin : " << this->xMin << std::endl;
    std::cout << "yMin : " << this->yMin << std::endl;
    std::cout << "xMan : " << this->xMax << std::endl;
    std::cout << "yMax : " << this->yMax << std::endl;
}


AABB::Tree::Tree()
{
}


AABB::Tree::~Tree()
{
}

void AABB::Tree::init(std::vector<AABB::Triangle>& triArray)
{

    // Copy triangles to the container of this class
    // This is master container of all triangle pointers
    this->triangleCountainer = triArray;

    std::vector<Triangle*> triPtrArray;
    triPtrArray.reserve(triangleCountainer.size());

    for (size_t i=0; i<triangleCountainer.size(); i++) {
        triPtrArray.push_back(&triangleCountainer[i]);
    }

    this->nodeContainer.reserve(triArray.size() * 2);

    // Build node tree recursively
    root = createNode(triPtrArray);
}

AABB::Node* AABB::Tree::createNode(std::vector<Triangle*>& triPtrArray)
{
    Node node;
    this->nodeContainer.push_back(node);
    Node* nodePtr = &(this->nodeContainer.back());

    nodePtr->triangles = triPtrArray;

    std::vector<float> xArray;
    std::vector<float> yArray;

    xArray.reserve(nodePtr->triangles.size() * 3);
    yArray.reserve(nodePtr->triangles.size() * 3);

    for (size_t i=0; i<nodePtr->triangles.size(); i++) {
        Triangle& t = *nodePtr->triangles[i];
        xArray.push_back(t.A.x);
        xArray.push_back(t.B.x);
        xArray.push_back(t.C.x);
       
        yArray.push_back(t.A.y);
        yArray.push_back(t.B.y);
        yArray.push_back(t.C.y);
    }

    float xMin = *std::min_element(xArray.begin(), xArray.end());
    float xMax = *std::max_element(xArray.begin(), xArray.end());
    float yMin = *std::min_element(yArray.begin(), yArray.end());
    float yMax = *std::max_element(yArray.begin(), yArray.end());

    nodePtr->aabb[0][0] = xMin;
    nodePtr->aabb[0][1] = yMin;
    nodePtr->aabb[1][0] = xMax;
    nodePtr->aabb[1][1] = yMax;
    nodePtr->xMin = xMin;
    nodePtr->xMax = xMax;
    nodePtr->yMin = yMin;
    nodePtr->yMax = yMax;

    nodePtr->width = xMax - xMin;
    nodePtr->height = yMax - yMin;

    if (nodePtr->triangles.size() < 3) {
        nodePtr->isLeaf = true;
        return nodePtr;
    } else {
        nodePtr->isLeaf = false;
        if (nodePtr->width < nodePtr->height) {
            std::sort(
                nodePtr->triangles.begin(),
                nodePtr->triangles.end(),
                [](const Triangle* t1, const Triangle* t2){return t1->medianPoint.y < t2->medianPoint.y;}); 
        } else {
            std::sort(
                nodePtr->triangles.begin(),
                nodePtr->triangles.end(),
                [](const Triangle* t1, const Triangle* t2){return t1->medianPoint.x < t2->medianPoint.x;}); 
        }
        std::vector<Triangle*> leftVec(nodePtr->triangles.begin(), nodePtr->triangles.begin() + nodePtr->triangles.size() / 2);
        std::vector<Triangle*> rightVec(nodePtr->triangles.begin() + nodePtr->triangles.size() / 2, nodePtr->triangles.end());

        nodePtr->left = createNode(leftVec);
        nodePtr->right = createNode(rightVec);

        return nodePtr;
    }
}

void AABB::nodeCompare(AABB::Node* base, AABB::Node* node, std::vector<int>& ot)
{
    if (node->isLeaf) {
        ot.push_back(1);
        return;
    }

    if ((*base) * (*node)) {
        AABB::nodeCompare(base, node->left, ot);
        AABB::nodeCompare(base, node->right, ot);
    }
}
