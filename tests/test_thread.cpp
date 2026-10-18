#undef NDEBUG
#include "thread/taskmgr.h"
#include "thread/mutex.h"
#include <atomic>
#include <cassert>
#include <cstdio>

using namespace jet::thread;

struct Count : Task {
    std::atomic<int>* n;
    int steps = 3;
    explicit Count(std::atomic<int>* p) : n(p) {}
    bool Run() override { ++*n; return --steps == 0; }
};

int main()
{
    TaskMgr mgr;
    mgr.Init();
    assert(TaskMgr::GetInstance() == &mgr);
    std::atomic<int> n(0);
    auto a = std::make_shared<Count>(&n);
    unsigned id = mgr.AddTask(a, TypeDef::MAIN);
    assert(id && a->IsFinished() && n == 3 && !mgr.HasTask(id));
    std::shared_ptr<Count> tasks[16];
    for (auto& t : tasks)
        t = std::make_shared<Count>(&n), mgr.AddTask(t, TypeDef::type(TypeDef::USER1 + (&t - tasks) % 4));
    mgr.FinishAllTasks();
    for (auto& t : tasks)
        assert(t->IsFinished());
    assert(n == 3 + 16 * 3 && mgr.CrtThreadHasType(TypeDef::RENDERING) && !mgr.CrtThreadHasType(TypeDef::USER2));
    RecursiveMutex rm;
    {
        ScopedMutex a1(rm), a2(rm);
        assert(rm.GetLockCount() == 2);
    }
    puts("thread ok");
}
