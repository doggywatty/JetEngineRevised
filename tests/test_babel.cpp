#undef NDEBUG
// Localization: .texts loading, {{references}}, the French filter, the number/date formatter.
#include "localization/babel.h"
#include "localization/formatter.h"
#include "localization/stringmgr.h"
#include "stream/streammgr.h"
#include <cassert>
#include <cstdio>
#include <string>

using namespace jet;

int main(int argc, char **argv) {
    std::string data = argc > 1 ? argv[1] : ".";
    stream::StreamMgr &sm = stream::StreamMgr::GetInstance();
    for (const char *d : { "text", "localizationText" }) sm.AddPath(String((data + "/" + d).c_str()), String());
    babel::Babel b;
    assert(b.SelectLanguage(babel::Lang::EN, true));
    babel::StringMgr *mgr = b.GetStringMgr();
    printf("CHEATS=%s\n", mgr->Get(String("CHEATS")).c_str());
    assert(mgr->Get(String("CHEATS")).Equals("Cheats"));
    assert(mgr->Get(String("no_such_key_x")).Equals("???"));
    assert(mgr->Exists(String("cheats")));
    babel::Formatter *f = b.GetFormatter();
    String n = f->FormatMeasure(1234567.25f, 2);
    printf("measure %s\n", n.c_str());
    assert(n.Equals("1,234,567.25"));
    assert(f->FormatMeasure(-1000).Equals("-1,000"));
    assert(f->FormatMeasure(42).Equals("42"));
    printf("percent %s crono %s\n", f->FormatPercentage(12.5f, 1).c_str(), f->FormatCrono(83456ull).c_str());
    assert(f->FormatCrono(83456ull).Equals("01:23:456"));
    printf("atoms %s | %s\n", f->FormatByAtoms(93784, 2).c_str(), f->FormatByAtomsVerbose(93784, 2).c_str());
    assert(b.SelectLanguage(babel::Lang::FR, true));
    printf("fr CHEATS=%s upper=%s\n", mgr->Get(String("CHEATS")).c_str(), f->ToUpper(String("\xc3\xa9t\xc3\xa9")).c_str());
    assert(f->FormatMeasure(1234567).Equals("1 234 567"));
    puts("babel ok");
    return 0;
}
