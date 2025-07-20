#include "grid.h"

template<typename T, int dimen>
int Grid<T,dimen>::posToIndex(const Vector2& pos)
{
    return (pos.y/dimen)*(area.width/dimen) + pos.x/dimen;
}

template<typename T, int x>
void Grid<T, x>::add(std::shared_ptr<T>& ptr)
{

}
