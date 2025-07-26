#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED

#include <memory>
#include <vector>
#include <iostream>
#include <unordered_map>

#include <raylib.h>
#include <raymath.h>
    int screenWidth = 1920;
    int screenHeight = 1080;
struct Circle
{
    static int count;
    Vector2 pos = {0,0};
    int radius = 1;
    Vector2 dir = {0,0};
    int id = 0;
    Color color = RED;

    Circle(Vector2 pos_, int radius_, Vector2 dir_, int id_, Color color_) : pos(pos_), radius(radius_), dir(dir_), id(id_), color(color_)
    {

    }
    ~Circle()
    {
        std::cout << "DELETED\n";
    }

    bool collidesWith(const Rectangle& rect)
    {
        return CheckCollisionCircleRec(pos,radius,rect);
    }
    bool collidesWith(const Vector2& point, float radius)
    {
        return (Vector2DistanceSqr(point,pos) <= pow(radius + this->radius,2));
    }


    void update()
    {
        pos += dir*GetFrameTime();
        if (pos.y > screenHeight)
        {
            pos.y = 0;
        }
        else if (pos.y < 0)
        {
            pos.y = screenHeight;
        }
        if (pos.x > screenWidth)
        {
            pos.x = 0;
        }
        else if (pos.x < 0)
        {
            pos.x = screenWidth;
        }
    }
};

int Circle::count = 0;


typedef int NodeIndex;

static constexpr NodeIndex NullNode = -1;

template<typename T>
struct SmartVector
{
    union Element
    {
        T t;
        int next;
    };
    int add(T t)
    {
        if (free == -1)
        {
            data.push_back({t});
            return data.size() - 1;
        }
        else
        {
            int oldFree = free;
            free = data[free].next;
            data[oldFree].t = t;
            return oldFree;
        }
    }
    void erase(int index)
    {
        data[index].next = free;
        free = index;
    }
    T& operator[](int index)
    {
        return data[index].t;
    }
    size_t size()
    {
        return data.size();
    }
private:
    std::vector<Element> data;
    int free = -1;
};


template<typename Thing, int capacity = 1000, int maxDepth = 10>
class QuadTree
{

    struct Node
    {
        //index in "nodes" of each child
        //if node is a parent, these are the indicies in "nodes" of each child
        //if node is a leaf, child[0] is the size, child[1] is the index of the first element (-1 if empty), child[2] and child[3] are -1
        int child[4] = {0,-1,-1,-1};
        bool isLeaf() const
        {
            return child[2] == -1;
        }
        int getSize() const
        {
            return child[0];
        }
        int getStart() const
        {
            return child[1];
        }
        void setStart(int start)
        {
            child[1] = start;
        }
        void addToSize(int amount) //should only be called on leaves
        {
            child[0] += amount;
        }
    };

    struct Element
    {
        //int index = 0; //index in masterList of the Thing this element represents
        Thing* index = 0;
        int next = -1; //index of next element in "elements" that belongs to the same node. -1 means end of the road
    };

    struct MasterListEntry
    {
        std::shared_ptr<Thing> ptr;
        int next= -1;

        Thing* operator->()
        {
            return ptr.get();
        }
    };

    std::unordered_map<Thing*,std::shared_ptr<Thing>> masterList;
    SmartVector<Node> nodes;
    SmartVector<Element> elements;

    Rectangle area;
    Node root;


    template<typename T>
    void forEachChild( T func, int index,Rectangle rect)
    {
        for (int i = 0; i < 4; i ++)
        {
            func(nodes[index].child[i],{rect.x + rect.width/2*(i%2),rect.y + rect.height/2*(i/2),rect.width/2,rect.height/2});
        }
    }

    template<typename T>
    void penetrate(const Rectangle& areaOfInterest, int index, Rectangle rect, T func)
    {
        penetrate([areaOfInterest](const Rectangle& rect){

                  return CheckCollisionRecs(areaOfInterest,rect);

                  },index,rect,func);
    }
    template<typename T>
    void penetrate(Thing& ptr, int index, Rectangle rect, T func)
    {
        penetrate([&ptr](const Rectangle& rect){

                  return ptr.collidesWith(rect);

                  },index,rect,func);
    }
    //collider = (Rectangle) => bool, true if we should process this child
    //run "func" on each node that returns true when passed into "collider
    template<typename T, typename Collider>
    void penetrate(Collider collider, int index, Rectangle rect, T func)
    {
        if (index < nodes.size())
        {
            if (!nodes[index].isLeaf())
            {
                forEachChild([this,func,collider](int nodeIndex, const Rectangle& nodeRect){
                            if (collider(nodeRect))
                            {
                                penetrate(collider,nodeIndex,nodeRect,func);
                            }
                             },index,rect);


            }
            else
            {
                func(index,rect);
            }
        }
    }

    //runs func on each element
    //func = (Element&, int) => void
    //func should ideally not change the element
    template<typename T>
    void forEachElement(int nodeIndex, T func)
    {
        int start = nodes[nodeIndex].getStart();
        while (start != -1)
        {
            int next = elements[start].next;
            func(elements[start],start);
            start = next;
        }
    }

    //move an existing element to a node
    void moveElement(int index, int elemIndex)
    {

        elements[elemIndex].next = nodes[index].getStart(); //set the next of new element = the old start
        nodes[index].setStart(elemIndex); //set the start = the new element
        nodes[index].addToSize(1);
    }

    void add(std::shared_ptr<Thing>& smartPtr, int index, Rectangle rect, int depth)
    {
        if (!nodes[index].isLeaf())
        {
            forEachChild([this,&smartPtr,depth](int nodeIndex,const Rectangle& nodeRect){

                        Node& child = nodes[nodeIndex];
                        if (smartPtr->collidesWith(nodeRect))
                        {
                            add(smartPtr,nodeIndex,nodeRect, depth + 1);
                        }
                         },index,rect);


        }
        else if (smartPtr->collidesWith(rect))
        {
            int elemIndex = elements.add({smartPtr.get()});

            moveElement(index,elemIndex);

                //    std::cout << nodes[index].getSize() << " " << index << "\n";
            if (nodes[index].getSize() > capacity && depth < maxDepth)
            {
                int size = nodes[index].getSize();
                int start = nodes[index].getStart();
                for (int i = 0; i < 4; i ++)
                {
                    nodes.add({});
                    nodes[index].child[i] = nodes.size()-1;
                }
                for (int i = 0; i < size; i ++)
                {
                    bool added = false;
                    int next = elements[start].next;
                    forEachChild([this,&added,&start](int nodeIndex, const Rectangle& nodeRect){
                                 if (masterList[elements[start].index]->collidesWith(nodeRect))
                                 {
                                     if (added) //if this element overlaps with two or more children, create a new element
                                     {
                                        int index = elements.add({elements[start].index,-1});
                                        moveElement(nodeIndex,index);
                                     }
                                     else //if this element is being added to a child for the first time, just move it from the parent
                                     {
                                         added = true;
                                         moveElement(nodeIndex,start);
                                     }
                                 }
                                 },index,rect);
                    start = next;
                }
            }

        }
    }

public:
    QuadTree(const Rectangle& rect) : area(rect)
    {
        nodes.add({});
    }
    void add(std::shared_ptr<Thing>& smartPtr)
    {
       // masterList.push_back(entry);
        if (smartPtr.get() && smartPtr->collidesWith(area))
        {
            masterList[smartPtr.get()] = smartPtr;
            add(smartPtr,0,area,0);
        }
    }
    void clear()
    {
        masterList.clear();
    }
    void remove(Thing& ptr)
    {
        auto it = masterList.find(&ptr);
        if (it != masterList.end())
        {
            penetrate(ptr,0,area,[&ptr,this](int nodeIndex, const Rectangle& rect){

            int next = nodes[nodeIndex].getStart();
            int prev = -1;
            while(next != -1)
            {
                if (elements[next].index == &ptr)
                {
                    if (prev == -1)
                    {
                        nodes[nodeIndex].setStart(elements[next].next);
                    }
                    else
                    {
                        elements[prev].next = elements[next].next;
                    }
                    nodes[nodeIndex].addToSize(-1);
                    break;
                }
                prev = next;
                next = elements[next].next;
            }

                      });

            /*std::cout << "HELLO\n";
            std::cout << (masterList.find(&ptr) != masterList.end()) << "\n";
            std::cout <<  &ptr << " " << masterList[&ptr].ptr.get()<< "\n";*/
            masterList.erase(it);
        }
    }

    //run func on each element in the radius
    //func = (Thing&) => void
    template<typename T>
    void forEachNearest(const Vector2& center, float radius, T func)
    {
        int nearest = 0;
        penetrate([center,radius](const Rectangle& rect){

                  return CheckCollisionCircleRec(center,radius,rect);

                  },0,area,[this,radius,center,func,&nearest](int nodeIndex, const Rectangle& nodeRect){
                      nearest += nodes[nodeIndex].getSize();
                  forEachElement(nodeIndex,[this,radius,center,func](Element& elem, int elemIndex){

                                 if (masterList[elem.index]->collidesWith(center,radius))
                                 {
                                     func(*elem.index);
                                 }

                                 });
                  });
        //std::cout << nearest << "\n";
    }

    void renderElements()
    {
        for (auto it = masterList.begin(); it != masterList.end(); ++it)
        {
                DrawPoly(it->first->pos,10,it->first->radius,0,RED);
                DrawText(std::to_string(it->first->id).c_str(),it->first->pos.x,it->first->pos.y,20,BLACK);
        }
    }

    void render()
    {

        penetrate(area,0,area,[this]( int nodeIndex,const Rectangle& nodeRect){

                  DrawRectangleLinesEx(nodeRect,1,BLACK);

                /*if (nodes[nodeIndex].isLeaf()){
                  DrawText(std::to_string(nodes[nodeIndex].getSize()).c_str(),nodeRect.x + 10, nodeRect.y + 10,20,BLACK);
                  DrawText(std::to_string(nodeIndex).c_str(),nodeRect.x + nodeRect.width/3, nodeRect.y + nodeRect.height/3,20,GREEN);
                    //std::cout << nodeIndex << "\n";
                  }*/

                  });
                if (IsKeyPressed(KEY_SPACE))
                {
                    std::cout << "-----------------------------------------\n";
                    for (int i = 0; i < nodes.size(); i ++)
                    {
                        if (nodes[i].isLeaf())
                        {
                            std::cout << "NODE " << i << " " << nodes[i].getStart()<< " :\n";

                            forEachElement(i,[this](Element& elem, int elementIndex){

                                            std::cout << "\t" << masterList[elem.index]->id<< " " << elementIndex << "\n";

                                           });
                        }
                    }
                }
    }

};

#endif // TREE_H_INCLUDED
