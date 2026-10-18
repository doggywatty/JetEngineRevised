// core/jet/include/core/protectedstorage.h
#ifndef CORE_JET_INCLUDE_CORE_PROTECTEDSTORAGE_H
#define CORE_JET_INCLUDE_CORE_PROTECTEDSTORAGE_H

#include <cstring>
#include <deque>
#include "jtypes.h"

namespace jet {
namespace core {

// Key and rotation used to scramble protected values (0xbaadf00d and 18 in the shipped game).
extern u32 s_protectedStorageKey;
extern u32 s_protectedStorageRotateBits;

// Anti memory-edit storage: the value is kept rotated and XOR-ed, with a byte check copy and the
// last 8 encoded values as history.
template <class T>
class ProtectedStorage {
public:
    struct Value {
        u8 bytes[8];
    };

    ProtectedStorage() { SetValue(T()); }
    ProtectedStorage(T v) { SetValue(v); }
    ProtectedStorage(const ProtectedStorage& o) { *this = o; }

    ProtectedStorage& operator=(T v) { SetValue(v); return *this; }
    ProtectedStorage& operator=(const ProtectedStorage& o)
    {
        m_value = o.m_value;
        for (int i = 0; i < 8; ++i)
            m_checkValue.bytes[i] = u8(i) ^ m_value.bytes[i];
        m_samples = o.m_samples;
        return *this;
    }

    void SetValue(T v)
    {
        EncodeValue(v, m_value);
        for (int i = 0; i < 8; ++i)
            m_checkValue.bytes[i] = u8(i) ^ m_value.bytes[i];
        m_samples.push_back(m_value);
        if (m_samples.size() > kSamples)
            m_samples.pop_front();
    }
    T GetValue() const { return DecodeValue(m_value); }
    operator T() const { return GetValue(); }
    bool IsValid() const
    {
        for (int i = 0; i < 8; ++i)
            if (m_checkValue.bytes[i] != (u8(i) ^ m_value.bytes[i]))
                return false;
        return true;
    }
    unsigned GetSampleCount() const { return unsigned(m_samples.size()); }
    T GetSample(unsigned i) const { return DecodeValue(m_samples[i]); }

    ProtectedStorage operator+(T v) const { return ProtectedStorage(GetValue() + v); }
    ProtectedStorage operator-(T v) const { return ProtectedStorage(GetValue() - v); }
    ProtectedStorage operator*(T v) const { return ProtectedStorage(GetValue() * v); }
    ProtectedStorage operator/(T v) const { return ProtectedStorage(GetValue() / v); }
    ProtectedStorage& operator+=(T v) { SetValue(GetValue() + v); return *this; }
    ProtectedStorage& operator-=(T v) { SetValue(GetValue() - v); return *this; }
    ProtectedStorage& operator*=(T v) { SetValue(GetValue() * v); return *this; }
    ProtectedStorage& operator/=(T v) { SetValue(GetValue() / v); return *this; }

protected:
    static const unsigned kSamples = 8;
    static u32 Encode(u32 v)
    {
        u32 r = s_protectedStorageRotateBits & 31;
        return (r ? (v << r | v >> (32 - r)) : v) ^ s_protectedStorageKey;
    }
    static u32 Decode(u32 v)
    {
        u32 r = s_protectedStorageRotateBits & 31;
        v ^= s_protectedStorageKey;
        return r ? (v >> r | v << (32 - r)) : v;
    }
    // values up to 32 bits are one encoded word, a double two (the value is 8 bytes)
    static void EncodeValue(T v, Value& out)
    {
        static_assert(sizeof(T) <= 8, "ProtectedStorage holds up to 64-bit values");
        memset(&out, 0, sizeof(out));
        u32 raw[2] = { 0, 0 };
        memcpy(raw, &v, sizeof(T));
        raw[0] = Encode(raw[0]);
        memcpy(out.bytes, &raw[0], 4);
        if (sizeof(T) > 4) {
            raw[1] = Encode(raw[1]);
            memcpy(out.bytes + 4, &raw[1], 4);
        }
    }
    static T DecodeValue(const Value& in)
    {
        u32 raw[2] = { 0, 0 };
        memcpy(&raw[0], in.bytes, 4);
        raw[0] = Decode(raw[0]);
        if (sizeof(T) > 4) {
            memcpy(&raw[1], in.bytes + 4, 4);
            raw[1] = Decode(raw[1]);
        }
        T v;
        memcpy(&v, raw, sizeof(T));
        return v;
    }

private:
    Value m_value;
    Value m_checkValue;
    std::deque<Value> m_samples;
};

}
}

#endif
