#pragma once

inline OOBB oobb(const Matrix4x4 &tm, const Vector3 &min, const Vector3 &max);

/// Creates an OOBB with no extent.
inline OOBB oobb(const Matrix4x4 &tm);

/// Converts OOBB coordinates (0-1, 0-1, 0-1) to world coordinates.
inline Vector3 transform(const OOBB &a, const Vector3 &p);

/// Returns the position of the i'th corner of the OOBB
inline Vector3 corner(const OOBB &a, unsigned i);

/// Extends the OOBB @a a with the point p (in world coordinates) and returns
/// the resulting OOBB. @a tminv should be the inverse of a.tm.
/// (It is passed for efficiency, in case it needs to be reused.)
inline OOBB extend(const OOBB &a, const Matrix4x4 &tminv, const Vector3 &p);

/// Extends the OOBB @a a with the OOBB @a b. @a tminv should be the inverse
/// of @a a.tm.
inline OOBB extend(const OOBB &a, const Matrix4x4 &tminv, const OOBB &b);

/// Centers the OOBB @a so that a.min = -a.max and returns the result.
inline OOBB center(const OOBB &a);

/// Returns true if the OOBB is empty.
inline bool empty(const OOBB &a);

#include "oobb.inl"
