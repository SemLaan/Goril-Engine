#include <goril.h>

#include "game/gameapp.h"


int main()
{
    Goril::App* app = new ExampleGame();
    app->Run();
    delete app;
}
