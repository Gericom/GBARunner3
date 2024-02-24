#pragma once
#include "Gui/OamManager.h"
#include "Gui/Animation/Animator.h"

class SplashScreen
{
public:
    void Initialize();
    void VBlank();
    bool IsFinished();

private:
    int _frame = 0;
    OamManager _mainOamManager;
    Animator<int> _gbarunnerTextAnimator;
    Animator<int> _threeGlowAnimator;
};
