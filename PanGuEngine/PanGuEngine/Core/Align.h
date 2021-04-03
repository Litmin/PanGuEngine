#pragma once

template <typename T>
bool IsPowerOfTwoD(T val)
{
    return val > 0 && (val & (val - 1)) == 0;
}

template <typename T>
inline T Align(T val, T alignment)
{
    if (!IsPowerOfTwoD(alignment))
        LOG_ERROR("Alignment must be power of two.");

    return (val + (alignment - 1)) & ~(alignment - 1);
}

template <typename T>
inline T AlignDown(T val, T alignment)
{
    if (!IsPowerOfTwoD(alignment))
        LOG_ERROR("Alignment must be power of two.");

    return val & ~(alignment - 1);
}