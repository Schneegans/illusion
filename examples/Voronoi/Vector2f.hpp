////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef VECTOR2F_HPP_INCLUDED
#define VECTOR2F_HPP_INCLUDED

#include <iostream>

/// A struct representing a 2D-Vector.
/// It provides more math functions then the SFML Vector
/// and is more optimized for MARS.

struct Vector2f {
  /// Default ctor (0, 0).
  Vector2f();
  /// Copy ctor.
  Vector2f(Vector2f const& point);
  /// Ctor from a \a x and a \a y value.
  Vector2f(double x, double y);

  /// Sets the length of the vector to 1.
  Vector2f normalize() const;

  /// Returns the length of the vector.
  /// Use Vector2f::lengthSquare for comparing the length
  /// of vectors, because it's much faster.
  double length() const;

  /// Returns the squared length of the vector.
  double lengthSquare() const;

  /// Overload for the operator += with another vector.
  Vector2f& operator+=(Vector2f const& rhs);

  /// Overload for the operator -= with another vector.
  Vector2f& operator-=(Vector2f const& rhs);

  /// Overload for the operator *= with a scalar.
  Vector2f& operator*=(double rhs);

  /// Overload for the operator /= with a scalar.
  Vector2f& operator/=(double rhs);

  /// \name Data
  /// Members storing the information of the vector.
  ///@{
  double x, y;
  ///@}
};

/// Addition of two vectors.
Vector2f operator+(Vector2f const& lhs, Vector2f const& rhs);

/// Subtraction of two vectors.
Vector2f operator-(Vector2f const& lhs, Vector2f const& rhs);

/// Scalar multiplication of two vectors.
double operator*(Vector2f const& lhs, Vector2f const& rhs);

/// Multiplication of a vector with a scalar.
Vector2f operator*(Vector2f const& lhs, double rhs);

/// Multiplication of a scalar with a vector.
Vector2f operator*(double const& lhs, Vector2f rhs);

/// Division of a vector by a scalar.
Vector2f operator/(Vector2f const& lhs, double rhs);

/// Comparision of two vectors.
bool operator==(Vector2f const& lhs, Vector2f const& rhs);

/// Comparision of two vectors.
bool operator!=(Vector2f const& lhs, Vector2f const& rhs);

/// Comparision of two vectors.
bool operator<(Vector2f const& lhs, Vector2f const& rhs);

/// Comparision of two vectors.
bool operator>(Vector2f const& lhs, Vector2f const& rhs);

/// Returns true, if second vector is rotated clockwise in reference to the first
bool clockWise(Vector2f const& first, Vector2f const& second);

/// Stream operator for a vector.
/// Creates an output like [x, y].
std::ostream& operator<<(std::ostream& os, Vector2f const& rhs);

#endif // VECTOR2F_HPP_INCLUDED
