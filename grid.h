#ifndef GRID_H_INCLUDED
#define GRID_H_INCLUDED

#include <list>
#include <iterator>
#include <memory>
#include <vector>

#include <raylib.h>

template<typename T, int dimen>
class Grid
{
    Rectangle area;

    typedef std::list<std::shared_ptr<T>> MasterList;

    MasterList masterList;
    std::vector<int> nodes;
    int posToIndex(const Vector2& pos);
public:
    Grid(const Rectangle& area_);
    void add(std::shared_ptr<T>& ptr);

};

#endif // GRID_H_INCLUDED
