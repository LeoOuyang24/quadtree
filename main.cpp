/*******************************************************************************************
*   raylib [core] example - Basic window
*
*   Welcome to raylib!
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "tree.h"
#include "grid.h"



int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------


    size_t seed = 1752944409;//time(NULL);
    srand(seed);
    std::cout << "SEED: " << seed << "\n";

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    QuadTree<Circle,100,5> tree({0,0,screenWidth,screenHeight});
    Grid<Circle,100,static_cast<int>(ceil(screenWidth/100.0f)),{0,0}> grid;

    std::vector<std::shared_ptr<Circle>> circles;

    for (int i = 0; i < 10000; i ++)
    {
        Circle::count ++;
        circles.emplace_back(new Circle({rand()%screenWidth,rand()%screenHeight},10,{rand()%100 - 50/50.0,rand()%100 - 50/50.0},Circle::count,Color(rand()%255,rand()%255,rand()%255,255)));
        //tree.add(circles[circles.size()-1]);
    }


    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Circle::count ++;
            circles.emplace_back(new Circle(GetMousePosition(),10,{rand()%100 - 50/50.0,rand()%100 - 50/50.0},Circle::count,RED));
            //tree.add(circles[circles.size()-1]);
            grid.add(circles[circles.size()-1]);
            std::cout << "CIRCLES: " << circles.size() << "\n";

        }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            tree.forEachNearest(GetMousePosition(),10,[&tree](Circle& thing){
                                std::cout << "ERASING: " << thing.id << "\n";
                                tree.remove(thing);
                                });
        }
        else if (IsMouseButtonDown((MOUSE_BUTTON_MIDDLE)))
        {
            Circle::count ++;
            circles.emplace_back(new Circle({rand()%screenWidth,rand()%screenHeight},10,{rand()%100 - 50/50.0,rand()%100 - 50/50.0},Circle::count,RED));
                    std::cout << "CIRCLES: " << circles.size() << "\n";

        }


        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();


            ClearBackground(RAYWHITE);
        for (int i = 0; i < circles.size(); i ++)
        {
            //tree.remove(*circles[i].get());
            grid.remove(*circles[i].get());

            circles[i]->update();
            DrawPoly(circles[i]->pos,6,circles[i]->radius,0,circles[i]->color);
            grid.add(circles[i]);

            grid.forEachNearest(circles[i]->pos,circles[i]->radius,[self=circles[i].get()](Circle& thing){
                                if (self != &thing)
                                {
                                    self->color = thing.color;
                                    //self->dir = Vector2Normalize(thing.pos - self->pos)*-10;
                                }

                                });

            //DrawCircle(circles[i]->pos.x,circles[i]->pos.y,circles[i]->radius,RED);
            //tree.add(circles[i]);



            /*tree.forEachNearest(circles[i]->pos,circles[i]->radius,[self=circles[i].get()](Circle& thing){
                                if (self != &thing)
                                {
                                    self->color = thing.color;
                                    //self->dir = Vector2Normalize(thing.pos - self->pos)*-10;
                                }

                                });*/

        }
        //grid.debug();

            //tree.renderElements();
            //tree.render();
            DrawFPS(10,10);


        EndDrawing();
        //----------------------------------------------------------------------------------
    }


    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
