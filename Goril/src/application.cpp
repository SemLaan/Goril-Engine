#include <goril.h>

#include "game/examplegame.h"


int main()
{
    Goril::AppProperties appProperties = {};
    appProperties.m_width = 600;
    appProperties.m_height = 300;

    Goril::App* app = new ExampleGame(appProperties);
    app->Run();
    delete app;
}
