#ifndef RESOURCES_H_INCLUDED
#define RESOURCES_H_INCLUDED

#include <raylib.h>
#include <raymath.h>

constexpr int screenWidth = 1920;
constexpr int screenHeight = 1080;

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
    Rectangle getBoundingRect()
    {
        return {pos.x - radius, pos.y - radius, radius*2, radius*2};
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

#endif // RESOURCES_H_INCLUDED
