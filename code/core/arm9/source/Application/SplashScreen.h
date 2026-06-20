#pragma once
#include "Gui/OamManager.h"
#include "Gui/Animation/Animator.h"

class SplashScreen
{
public:
    void Initialize();
    void VBlank();
    bool IsFinished();
    void EnterBusyLoop();
    void ExitBusyLoop();

private:
    int _frame = 0;
    volatile bool _busy = false;
    OamManager _mainOamManager;
    Animator<int> _gbarunnerTextAnimator;
    Animator<int> _threeGlowAnimator;
};
