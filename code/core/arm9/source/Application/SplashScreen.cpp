#include "common.h"
#include <algorithm>
#include <libtwl/gfx/gfx.h>
#include <libtwl/gfx/gfxWindow.h>
#include <libtwl/gfx/gfxBackground.h>
#include <libtwl/gfx/gfxOam.h>
#include <libtwl/sys/sysPower.h>
#include "Gui/Animation/LinearCurve.h"
#include "Gui/OamBuilder.h"
#include "SplashScreen.h"

#define ANIM_GBARUNNER_FULLY_SHOWN_FRAMES   24
#define ANIM_GBARUNNER_TOTAL_FRAMES         105
#define GLOW_ALPHA_MAX                       16
#define GLOW_RISE_FRAMES                     16
#define GLOW_FALL_FRAMES                      6
// Slower than the one-shot intro pulse above: this one repeats indefinitely while busy,
// so a fast cycle reads as frantic in a way a single intro pulse never does.
#define BUSY_GLOW_RISE_FRAMES                48
#define BUSY_GLOW_FALL_FRAMES                18

[[gnu::section(".ewram")]] void SplashScreen::EnterBusyLoop() { _busy = true; }

[[gnu::section(".ewram")]] void SplashScreen::ExitBusyLoop()
{
    _busy = false;
    // The busy-loop pulse can end mid-ramp at an arbitrary brightness; settle back to the
    // resting "no glow" state immediately instead of freezing on whatever value was last drawn.
    REG_BLDALPHA = GLOW_ALPHA_MAX << 8;
}

[[gnu::section(".ewram")]] void SplashScreen::Initialize()
{
    REG_BG0CNT = 0x0202;
    REG_BG1CNT = 0x0304;

    gfx_setWindow0(0, 82, 255, 110);
    REG_WININ = (1 << 4) | (1 << 1) | (1 << 5);
    REG_WINOUT = (1 << 0) | (1 << 1) | (1 << 4) | (1 << 5);

    REG_DISPCNT = 0x10000 | (1 << 8) | (1 << 9) | (1 << 12) | (1 << 4) | (1 << 13) | 0;

    REG_BLDALPHA = GLOW_ALPHA_MAX << 8;
    REG_BLDCNT = (1 << 8) | (1 << 13) | (1 << 12) | (1 << 1) | (1 << 6);

    sys_setMainEngineToTopScreen();

    _mainOamManager.Clear();
    _mainOamManager.Apply(GFX_OAM_MAIN);

    _frame = 0;
    _gbarunnerTextAnimator = Animator<int>(-43, 173, ANIM_GBARUNNER_FULLY_SHOWN_FRAMES, &gLinearCurve);
    _threeGlowAnimator = Animator<int>(0, GLOW_ALPHA_MAX, GLOW_RISE_FRAMES, &gLinearCurve);
}

[[gnu::section(".ewram")]] void SplashScreen::VBlank()
{
    if (IsFinished())
    {
        if (!_busy) return;

        // Keep the existing glow pulse oscillating indefinitely instead of letting
        // it settle at 0 once boot work outlasts the intro animation. Uses its own
        // (slower) timing than the intro's one-shot pulse, since a fast cycle reads
        // as frantic once it's repeating indefinitely instead of playing once.
        if (_threeGlowAnimator.Update())
        {
            int next = _threeGlowAnimator.GetValue() == 0 ? GLOW_ALPHA_MAX : 0;
            int duration = (next == GLOW_ALPHA_MAX) ? BUSY_GLOW_RISE_FRAMES : BUSY_GLOW_FALL_FRAMES;
            _threeGlowAnimator.Goto(next, duration, &gLinearCurve);
        }
        REG_BLDALPHA = _threeGlowAnimator.GetValue() | ((GLOW_ALPHA_MAX - _threeGlowAnimator.GetValue()) << 8);
        return;
    }

    _mainOamManager.Clear();
    _gbarunnerTextAnimator.Update();
    if (_frame < ANIM_GBARUNNER_FULLY_SHOWN_FRAMES + 16)
    {
        OamBuilder::OamWithSize<64, 32>(_gbarunnerTextAnimator.GetValue(), 83, 512 >> 5)
            .WithPalette16(1)
            .WithPriority(3)
            .Build(_mainOamManager.AllocOams(1)[0]);
    }
    else
    {
        OamBuilder::OamWithSize<32, 32>(189, 77, 0)
            .WithPalette16(0)
            .WithPriority(3)
            .Build(_mainOamManager.AllocOams(1)[0]);
    }
    if (_frame >= ANIM_GBARUNNER_FULLY_SHOWN_FRAMES)
    {
        if (_threeGlowAnimator.Update() && _threeGlowAnimator.GetValue() != 0)
        {
            _threeGlowAnimator.Goto(0, GLOW_FALL_FRAMES, &gLinearCurve);
        }
    }

    _mainOamManager.Apply(GFX_OAM_MAIN);
    gfx_setWindow0(std::max(0, _gbarunnerTextAnimator.GetValue() + 20), 82, 255, 110);
    if (_frame >= ANIM_GBARUNNER_FULLY_SHOWN_FRAMES)
    {
        REG_BLDALPHA = _threeGlowAnimator.GetValue() | ((GLOW_ALPHA_MAX - _threeGlowAnimator.GetValue()) << 8);
    }
    REG_MASTER_BRIGHT = 0;
    _frame++;
}

[[gnu::section(".ewram")]] bool SplashScreen::IsFinished()
{
    return _frame >= ANIM_GBARUNNER_TOTAL_FRAMES;
}
