#ifndef GRID_H_INCLUDED
#define GRID_H_INCLUDED

#include "resources.h"

template<typename T, int nodeDimen, int nodesPerWidth, Vector2 origin>
class Grid
{
    struct Lookup
    {
        std::shared_ptr<T> ptr;
        int first = -1;
    };
    struct Element
    {
        T* ptr = nullptr;
        int next = -1;
        int prev = -1; //if prev is negative, this element is the first of a node, that node being node[-1*element.prev + 1]
        int nextCopy = -1;
    };

    typedef int Node; //Nodes just contain the index of their first element, -1 if empty;

    std::unordered_map<T*,Lookup> masterList;
    SmartVector<Element> elements;
    std::vector<Node> nodes;

    int posToIndex(const Vector2& pos)
    {
        return static_cast<int>((pos.x - origin.x)/nodeDimen) + static_cast<int>((pos.y - origin.y)/nodeDimen)*nodesPerWidth;
    }
    Vector2 indexToPos(int index)
    {
        return origin + Vector2{index%nodesPerWidth*nodeDimen,index/nodesPerWidth*nodeDimen};
    }
    //F = (int nodeIndex) -> void
    template<typename F>
    void process(F func, const Rectangle& rect)
    {
        int start = posToIndex({rect.x,rect.y});
        int horizEnd = posToIndex({rect.x + rect.width, rect.y});
        int vertEnd =  posToIndex({rect.x, rect.y + rect.height});
        for (int i = 0; i <= horizEnd - start; i++)
        for (int j = 0; j <= (vertEnd - start)/nodesPerWidth; j ++)
            {
                func(start + i + j*nodesPerWidth);
            }
    }
    template<typename F>
    void processThing(F func, T& thing)
    {
        process(func,thing.getBoundingRect());
    }
    //run a function on each element in a node
    //F = (T&) => void
    template<typename F>
    void forEachElement(int nodeIndex, F func)
    {
        int current = nodes[nodeIndex];
        while (current != -1)
        {
            func(*elements[current].ptr);
            current = elements[current].next;
        }
    }

    void addElement(int nodeIndex, T& thing)
    {

        int index = elements.add({&thing,nodes[nodeIndex],-1*(nodeIndex + 1),masterList[&thing].first});
        //std::cout << nodes[nodeIndex] << " NEXT WHEN ADDING: " << elements[index].next << "\n";

        if (nodes[nodeIndex] != -1)
        {
            elements[nodes[nodeIndex]].prev = index;
        }

        nodes[nodeIndex] = index;
        masterList[&thing].first = index;
    }
public:
    void add(std::shared_ptr<T>& thing)
    {
        if (T* ptr = thing.get())
        if (masterList.find(ptr) == masterList.end())
        {
            Rectangle rect = ptr->getBoundingRect();
            if (rect.x >= origin.x && rect.y >= origin.y && rect.x <= origin.x + nodeDimen*nodesPerWidth) //thing must be to the bottom right of our origin to be added
            {
                masterList[ptr] = {thing};

                int endIndex = posToIndex({rect.x + rect.width, rect.y + rect.height});
                if (endIndex >= nodes.size()) //if thing is out of current range, add the necessary nodes
                {
                    nodes.resize(endIndex + 1, -1);
                }

                processThing([this,thing = thing.get()](int nodeIndex){

                            addElement(nodeIndex,*thing);

                             },*ptr);
            }
        }
    }
    void remove(T& thing)
    {
        if (masterList.find(&thing) != masterList.end())
        {
            int current = masterList[&thing].first;
            while (current != -1)
            {
                if (elements[current].prev >= 0)
                {
                    elements[elements[current].prev].next = elements[current].next;
                }
                else //element is first of a node
                {
                    nodes[(elements[current].prev + 1)*-1]= elements[current].next;
                }
                int old = elements[current].nextCopy;
                elements.erase(current);
                current = old;
            }
            masterList.erase(&thing);
        }
    }

    //run a function on each element in range
    //F = (T&) => void
    template<typename F>
    void forEachNearest(const Vector2& pos, int radius, F func)
    {
        process([func,this,pos,radius](int nodeIndex){
                forEachElement(nodeIndex,[pos,radius,func](T& t){

                               if (t.collidesWith(pos,radius))
                               {
                                   func(t);
                               }

                               });

                },{pos.x - radius, pos.y - radius, radius*2, radius*2});
    }
    void debug()
    {
        int overHang = nodes.size()%nodesPerWidth;
        int maxHeight = indexToPos(nodes.size()-1).y/nodeDimen + 1;
        int maxWidth = std::min(static_cast<int>(nodes.size()),nodesPerWidth);
        //std::cout << nodes.size() <<" " << maxWidth << " " << maxHeight << " "<< "\n";
        for (int i = 0; i < maxWidth+1; i ++)
        {
            DrawLine(i*nodeDimen,origin.y,i*nodeDimen,origin.y + (maxHeight + (i <= overHang ))*nodeDimen ,BLACK);
        }
        for (int i = 0; i < maxHeight+1; i ++)
        {
            DrawLine(origin.x,i*nodeDimen,origin.x + ((i == maxHeight) ? overHang : maxWidth)*nodeDimen,i*nodeDimen,BLACK);
        }

        for (int i = 0; i < nodes.size(); i ++)
        {
            Vector2 pos = indexToPos(i);
            int next = nodes[i];
            int count = 0;
            while (next != -1)
            {
                count ++;

                next = elements[next].next;
            }
            DrawText(std::to_string(count).c_str(),pos.x + nodeDimen/2,pos.y + nodeDimen/2,20,BLACK);
        }
    }

};

#endif // GRID_H_INCLUDED
