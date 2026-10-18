#undef NDEBUG
#include "core/pstring.h"
#include <cassert>
#include <cstdio>
#include <string>

using jet::String;

int main()
{
    String a("Hello"), b("hello"), c("HELLO"), d("World");
    assert(a == b && b == c && a != d);
    assert(!a.EqualsCS(b) && a.EqualsCS(String("Hello")));
    assert(std::string(b.c_str()) == "hello");
    assert(a.GetKey() == c.GetKey() && a.GetHash() == b.GetHash());
    assert(String().empty() && String("").empty() && String::null == "");
    String e = a + " " + d;
    assert(e == "hello world" && e.size() == 11);
    assert(e.find("WORLD") == 6 && e.find_last_of('o') == 7 && e.find_first_not_of("leh") == 4);
    assert(e.substr(6) == "World" && e.substr(0, 5).EqualsCS("Hello"));
    assert(e.GetLowercase().EqualsCS("hello world") && e.GetUppercase().EqualsCS("HELLO WORLD"));
    assert(String::Format("%d-%s", 42, "x") == "42-X");
    std::string big(5000, 'a');
    assert(String(big).size() == 5000 && String(big) == String(std::string(5000, 'A')));
    assert(String("abc") < String("ABD") && !(String("b") < String("A")));
    for (int i = 0; i < 5000; ++i)
        String::Format("tmp%d", i);
    assert(a == "HeLLo" && std::string(a.c_str()) == "Hello");
    puts("pstring ok");
}
