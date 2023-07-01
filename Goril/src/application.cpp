#include <goril.h>

#include "game/examplegame.h"


int main()
{
    Goril::App* app = new ExampleGame();
    app->Run();
    delete app;
}
