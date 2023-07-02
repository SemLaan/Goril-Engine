#pragma once
#include <goril.h>

#include "examplelayer.h"

class ExampleGame : public Goril::App
{
public:
    ExampleGame(Goril::AppProperties& appProperties)
        : App(appProperties)
    {
        PushLayer(Goril::CreateRef<ExampleLayer>());
    }

    ~ExampleGame()
    {

    }
};