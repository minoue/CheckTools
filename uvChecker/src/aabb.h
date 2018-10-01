#ifndef __AABB_H__
#define __AABB_H__

#include <string>
#include <vector>

namespace AABB {

class Point {
public:
    Point();
    Point(float x, float y);
    ~Point();

    float x, y;
    int index;
    std::string fullPath;
};

class Triangle {
public:
    Triangle();
    Triangle(Point a, Point b, Point c, std::string fullPath);
    ~Triangle();

    Point A, B, C;
    Point medianPoint;
    int index;
    int shellIndex;
    std::string fullPath;
};

class Node
{
    public:
        Node();
        ~Node();

        Node* left;
        Node* right;
        bool isLeaf;
        std::vector<Triangle*> triangles;

        float aabb[2][2];
        float width;
        float height;
        float xMin, xMax, yMin, yMax;
        bool operator*(const Node& rhs) const;

        void printAABB();

    private:
};

class Tree
{
public:
    Tree();
    ~Tree();
    AABB::Node* root;
    AABB::Node* createNode(std::vector<Triangle*>& triPtrArray);
    void init(std::vector<Triangle>& triArray);
private:
    std::vector<Node> nodeContainer;
    std::vector<Triangle> triangleCountainer;
};

void nodeCompare(Node* base, Node* node, std::vector<int>& ot);

} // Namespace ends

#endif /* defined(__AABB_H__) */
