#undef NDEBUG
#include "core/averager.h"
#include "core/clock.h"
#include "core/delayer.h"
#include "core/interpolator.h"
#include "core/protectedstorage.h"
#include "core/util.h"
#include "system.h"
#include "util/fps.h"
#include <cassert>
#include <cmath>
#include <cstdio>

using namespace jet;
using namespace jet::core;

int main(int argc, const char** argv)
{
    System::InitHeadless();
    System::ParseCommandLine(argc, argv);

    ProtectedStorage<int> coins(1000);
    coins += 25;
    assert(coins.GetValue() == 1025 && coins.IsValid() && coins.GetSampleCount() == 2);
    u32 raw;
    memcpy(&raw, &coins, 4);
    assert(raw != 1025);  // not stored in the clear
    ProtectedStorage<float> f(2.5f);
    assert(f.GetValue() == 2.5f);

    Delayer<float, SmoothStep> d(0.0f, 10.0f, 100);
    d.Update(50);
    assert(std::fabs(d.GetCurrentValue() - 5.0f) < 1e-4f && d.IsUpdating());
    d.Update(60);
    assert(d.GetCurrentValue() == 10.0f && !d.IsUpdating());

    Averager<float> a(3);
    for (float v : {1.0f, 2.0f, 3.0f, 10.0f})
        a.Push(v);
    assert(std::fabs(a.GetMean() - 5.0f) < 1e-5f && a.GetMax() == 10.0f);

    Interpolator<math::vec3<float>> path({{math::vec3<float>(0, 0, 0), 0}, {math::vec3<float>(10, 0, 0), 100}}, InterpolationTypeDef::LINEAR);
    assert(std::fabs(path.GetValueAtTime(25).x - 2.5f) < 1e-5f && path.GetValueAtTime(500).x == 10.0f);
    assert(GetInterpolated(InterpolationDef::QUADRATIC_EASE_IN, 0.5f) == 0.25f && GetInterpolated(InterpolationDef::STEP, 0.99f) == 0.0f);

    Srand(123456789, 362436069, 521288629, 88675123);
    int lo = 100, hi = 0;
    for (int i = 0; i < 1000; ++i) {
        int r = Rand(5, 10);
        lo = r < lo ? r : lo;
        hi = r > hi ? r : hi;
    }
    assert(lo >= 5 && hi <= 9 && hi > lo);
    u32 seed = 7;
    float rf = Randf2(seed);
    assert(rf >= -1.0f && rf < 1.0f);

    Clock c;
    util::Fps fps(100);
    fps.Tick();
    for (int i = 0; i < 5; ++i)
        fps.Tick();
    assert(fps.GetFps() > 50 && fps.GetFps() < 120 && c.GetElapsed() >= 40);
    assert(!System::GetUserFolder().empty() && System::GetMemoryPageAlignment() >= 1);
    printf("fps %.1f avg %.1f, clock %lld ms\n", fps.GetFps(), fps.GetAverageFps(), (long long)c.GetElapsed());
    System::Free();
    puts("coreutil ok");
}
