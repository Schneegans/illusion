////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Color.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////

Color::Color(std::string const& html) {
  htmlRgba(html);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Color::Color(float r, float g, float b, float a)
    : mVal(r, g, b, a) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float Color::r() const {
  return mVal.r;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float Color::g() const {
  return mVal.g;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float Color::b() const {
  return mVal.b;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float Color::h() const {

  if (s() > 0.0f) {
    float maxi = std::max(std::max(mVal.r, mVal.g), mVal.b);
    float mini = std::min(std::min(mVal.r, mVal.g), mVal.b);

    if (maxi == mVal.r)
      return std::fmod(60.f * ((mVal.g - mVal.b) / (maxi - mini)) + 360.f, 360.f);
    else if (maxi == mVal.g)
      return std::fmod(60.f * (2 + (mVal.b - mVal.r) / (maxi - mini)) + 360.f, 360.f);
    else
      return std::fmod(60.f * (4 + (mVal.r - mVal.g) / (maxi - mini)) + 360.f, 360.f);
  } else
    return 0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float Color::s() const {
  if (v() == 0)
    return 0;
  else
    return ((v() - std::min(std::min(mVal.r, mVal.g), mVal.b)) / v());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float Color::v() const {
  return std::max(std::max(mVal.r, mVal.g), mVal.b);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float Color::a() const {
  return mVal.a;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Color::r(float red) {
  mVal.r = glm::clamp(red, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Color::g(float green) {
  mVal.g = glm::clamp(green, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Color::b(float blue) {
  mVal.b = glm::clamp(blue, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Color::h(float hue) {
  setHsv(hue, s(), v());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Color::s(float saturation) {
  setHsv(h(), glm::clamp(saturation, 0.0f, 1.0f), v());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Color::v(float value) {
  setHsv(h(), s(), glm::clamp(value, 0.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Color::a(float alpha) {
  mVal[3] = glm::clamp(alpha, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Color::setHsv(float hue, float saturation, float value) {

  if (saturation == 0.0f) {
    mVal.r = value;
    mVal.g = value;
    mVal.b = value;
    return;
  }
  hue = std::fmod(hue, 360.0f);
  hue /= 60.0f;
  int32_t i = int32_t(std::floor(hue));
  float   f = hue - i;

  switch (i) {
  case 0:
    mVal.r = value;
    mVal.g = value * (1.0f - saturation * (1.0f - f));
    mVal.b = value * (1.0f - saturation);
    break;
  case 1:
    mVal.r = value * (1.0f - saturation * f);
    mVal.g = value;
    mVal.b = value * (1.0f - saturation);
    break;
  case 2:
    mVal.r = value * (1.0f - saturation);
    mVal.g = value;
    mVal.b = value * (1.0f - saturation * (1.0f - f));
    break;
  case 3:
    mVal.r = value * (1.0f - saturation);
    mVal.g = value * (1.0f - saturation * f);
    mVal.b = value;
    break;
  case 4:
    mVal.r = value * (1.0f - saturation * (1.0f - f));
    mVal.g = value * (1.0f - saturation);
    mVal.b = value;
    break;
  default:
    mVal.r = value;
    mVal.g = value * (1.0f - saturation);
    mVal.b = value * (1.0f - saturation * f);
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Color Color::inverted() const {

  Color inverted(*this);
  inverted.h(inverted.h() + 180.0f);
  if (v() < 0.5f)
    inverted.v(1.0f - inverted.v());
  return inverted;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Color Color::brightened() const {

  Color brightened(*this);
  if (brightened.v() < 0.5f)
    brightened.v(0.5f);
  if (brightened.s() < 0.5f)
    brightened.s(0.5f);
  return brightened;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::vec3 Color::vec3() const {
  return glm::vec3(mVal.x, mVal.y, mVal.z);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

glm::vec4 const& Color::vec4() const {
  return mVal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::string Color::htmlRgba() const {
  std::stringstream stream;
  stream << "rgba(" << static_cast<int32_t>(r() * 255.0f) << ", "
         << static_cast<int32_t>(g() * 255.0f) << ", " << static_cast<int32_t>(b() * 255.0f) << ", "
         << a() << ")";

  return stream.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Color::htmlRgba(std::string const& val) {
  std::string str(val);
  std::string cropped = str.substr(str.find_first_of("(") + 1, str.find_first_of(")") - 1);

  std::stringstream stream(cropped);
  std::string       comma;
  float             red;
  float             green;
  float             blue;
  stream >> red >> comma >> green >> comma >> blue >> comma;

  if (comma == ",") {
    float alpha;
    stream >> alpha;
    a(alpha);
  } else {
    a(1.f);
  }

  r(red / 255.f);
  g(green / 255.f);
  b(blue / 255.f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float Color::operator[](unsigned rhs) const {
  return mVal[rhs];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float& Color::operator[](unsigned rhs) {
  return mVal[rhs];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool operator==(Color const& lhs, Color const& rhs) {
  return lhs.r() == rhs.r() && lhs.g() == rhs.g() && lhs.b() == rhs.b() && lhs.a() == rhs.a();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool operator!=(Color const& lhs, Color const& rhs) {
  return !(lhs == rhs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Color operator*(float const& lhs, Color rhs) {
  return Color(rhs.r() * lhs, rhs.g() * lhs, rhs.b() * lhs, rhs.a());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Color operator*(Color const& lhs, float rhs) {
  return rhs * lhs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Color operator/(Color const& lhs, float rhs) {
  return Color(lhs.r() / rhs, lhs.g() / rhs, lhs.b() / rhs, lhs.a());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Color operator+(Color const& lhs, Color const& rhs) {
  Color result;
  result.r(lhs.r() + rhs.r());
  result.g(lhs.g() + rhs.g());
  result.b(lhs.b() + rhs.b());
  result.a(0.5f * (lhs.a() + rhs.a()));
  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Color operator-(Color const& lhs, Color const& rhs) {
  Color result;
  result.r(lhs.r() - rhs.r());
  result.g(lhs.g() - rhs.g());
  result.b(lhs.b() - rhs.b());
  result.a(0.5f * (lhs.a() + rhs.a()));
  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, Color const& color) {
  os << color.vec4();
  return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Core
