////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Vector2f.hpp"

#include <cmath>

Vector2f::Vector2f()
    : x(0.0)
    , y(0.0) {
}

Vector2f::Vector2f(Vector2f const& point)
    : x(point.x)
    , y(point.y) {
}

Vector2f::Vector2f(double x_in, double y_in)
    : x(x_in)
    , y(y_in) {
}

Vector2f Vector2f::normalize() const {
  double len = length();
  if (len != 0)
    return Vector2f(x / len, y / len);
  else
    return Vector2f(0, 0);
}

double Vector2f::length() const {
  return (std::sqrt(x * x + y * y));
}

double Vector2f::lengthSquare() const {
  return x * x + y * y;
}

Vector2f& Vector2f::operator+=(Vector2f const& rhs) {
  x += rhs.x;
  y += rhs.y;
  return *this;
}

Vector2f& Vector2f::operator-=(Vector2f const& rhs) {
  x -= rhs.x;
  y -= rhs.y;
  return *this;
}

Vector2f& Vector2f::operator*=(double rhs) {
  x *= rhs;
  y *= rhs;
  return *this;
}

Vector2f& Vector2f::operator/=(double rhs) {
  x /= rhs;
  y /= rhs;
  return *this;
}

Vector2f operator+(Vector2f const& lhs, Vector2f const& rhs) {
  return (Vector2f(lhs.x + rhs.x, lhs.y + rhs.y));
}

Vector2f operator-(Vector2f const& lhs, Vector2f const& rhs) {
  return (Vector2f(lhs.x - rhs.x, lhs.y - rhs.y));
}

double operator*(Vector2f const& lhs, Vector2f const& rhs) {
  return (lhs.x * rhs.x + lhs.y * rhs.y);
}

Vector2f operator*(Vector2f const& lhs, double rhs) {
  return (Vector2f(lhs.x * rhs, lhs.y * rhs));
}

Vector2f operator*(double const& lhs, Vector2f rhs) {
  return rhs * lhs;
}

Vector2f operator/(Vector2f const& lhs, double rhs) {
  return (Vector2f(lhs.x / rhs, lhs.y / rhs));
}

bool operator==(Vector2f const& lhs, Vector2f const& rhs) {
  return (lhs.x == rhs.x && lhs.y == rhs.y);
}

bool operator!=(Vector2f const& lhs, Vector2f const& rhs) {
  return (lhs.x != rhs.x && lhs.y != rhs.y);
}

bool operator<(Vector2f const& lhs, Vector2f const& rhs) {
  return lhs.lengthSquare() < rhs.lengthSquare();
}

bool operator>(Vector2f const& lhs, Vector2f const& rhs) {
  return lhs.lengthSquare() > rhs.lengthSquare();
}

bool clockWise(Vector2f const& first, Vector2f const& second) {
  return (first.x * second.y - first.y * second.x) < 0;
}

std::ostream& operator<<(std::ostream& os, Vector2f const& rhs) {
  return os << "[" << rhs.x << ", " << rhs.y << "]";
}
