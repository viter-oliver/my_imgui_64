#pragma once
#include "application.h"
class afg_engine : public auto_future::application
{
public:
    afg_engine(int argc, char **argv);
    void resLoaded();
    void onUpdate();
};