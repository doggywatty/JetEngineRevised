// core/base/include/jenum.h
#ifndef CORE_BASE_INCLUDE_JENUM_H
#define CORE_BASE_INCLUDE_JENUM_H

// Scoped enum wrapper: struct FooDef { enum type { A, B }; }; typedef safe_enum<FooDef> Foo;
// Storage is the in-memory representation (the enum itself, or e.g. unsigned char to save space).
template <class Def, class Storage = typename Def::type>
class safe_enum : public Def {
public:
    typedef typename Def::type type;

    safe_enum() : m_value(Storage(0)) {}
    safe_enum(type v) : m_value(Storage(v)) {}
    explicit safe_enum(int v) : m_value(Storage(v)) {}

    type underlying() const { return type(m_value); }
    operator type() const { return type(m_value); }

    bool operator==(const safe_enum& o) const { return m_value == o.m_value; }
    bool operator!=(const safe_enum& o) const { return m_value != o.m_value; }
    bool operator==(type v) const { return type(m_value) == v; }
    bool operator!=(type v) const { return type(m_value) != v; }
    bool operator<(const safe_enum& o) const { return m_value < o.m_value; }
    bool operator<=(const safe_enum& o) const { return m_value <= o.m_value; }
    bool operator>(const safe_enum& o) const { return m_value > o.m_value; }
    bool operator>=(const safe_enum& o) const { return m_value >= o.m_value; }

private:
    Storage m_value;
};

#endif
