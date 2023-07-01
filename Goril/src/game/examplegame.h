#pragma once
#include <goril.h>

#include "examplelayer.h"

class ExampleGame : public Goril::App
{
public:
    ExampleGame()
    {
        PushLayer(Goril::CreateRef<ExampleLayer>());
    }

    ~ExampleGame()
    {

    }
};