#pragma once

namespace TP {

template <typename T>
inline T abs(const T& x) {return (x < T{}) ? -x : x;}

template <typename T>
inline T diff(const T& lhs, const T& rhs) {
    if (lhs >= rhs)
        return lhs - rhs;
    else
        return rhs - lhs;
}

}