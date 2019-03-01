////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/Color.hpp>

#include <doctest.h>

namespace Illusion::Core {

TEST_CASE("Illusion::Core::Color") {

  SUBCASE("Checking html conversions") {
    Color color1("rgba(255, 127, 0, 0.2)");
    CHECK(color1.r() == 1.f);
    CHECK(color1.g() == 127.f / 255.f);
    CHECK(color1.b() == 0.f);
    CHECK(color1.a() == 0.2f);
    CHECK(color1.htmlRgba() == "rgba(255, 127, 0, 0.2)");

    Color color2("rgba(78, 127, 42)");
    CHECK(color2.r() == 78.f / 255.f);
    CHECK(color2.g() == 127.f / 255.f);
    CHECK(color2.b() == 42.f / 255.f);
    CHECK(color2.a() == 1.f);
    CHECK(color2.htmlRgba() == "rgba(78, 127, 42, 1)");
  }

  SUBCASE("Checking from-value constructor") {
    Color color(1.f, 0.5f, 0.3f, 0.7f);
    CHECK(color.r() == 1.f);
    CHECK(color.g() == 0.5f);
    CHECK(color.b() == 0.3f);
    CHECK(color.a() == 0.7f);
  }

  SUBCASE("Checking RGB setters") {
    Color color;
    color.setRGB(1.f, 0.5f, 0.3f, 0.7f);
    CHECK(color.r() == 1.f);
    CHECK(color.g() == 0.5f);
    CHECK(color.b() == 0.3f);
    CHECK(color.a() == 0.7f);
  }

  SUBCASE("Checking HSV getters") {
    Color red(1.f, 0.f, 0.f);
    CHECK(red.h() == 0.f);
    CHECK(red.s() == 1.f);
    CHECK(red.v() == 1.f);

    Color lime(0.5f, 1.0f, 0.5f);
    CHECK(lime.h() == 120.f);
    CHECK(lime.s() == 0.5f);
    CHECK(lime.v() == 1.f);

    Color purple(0.6f, 0.25f, 0.65f);
    CHECK(purple.h() == doctest::Approx(292.5f));
    CHECK(purple.s() == doctest::Approx(0.4f / 0.65f));
    CHECK(purple.v() == doctest::Approx(0.65f));

    Color white(1.f, 1.f, 1.f);
    CHECK(white.h() == 0.f);
    CHECK(white.s() == 0.f);
    CHECK(white.v() == 1.f);

    Color black(0.f, 0.f, 0.f);
    CHECK(black.h() == 0.f);
    CHECK(black.s() == 0.f);
    CHECK(black.v() == 0.f);
  }

  SUBCASE("Checking HSV setters") {
    Color red;
    red.setHSV(0.f, 1.f, 1.f);
    CHECK(red.r() == 1.f);
    CHECK(red.g() == 0.f);
    CHECK(red.b() == 0.f);
    CHECK(red.a() == 1.f);

    Color lime;
    lime.setHSV(120.f, 0.5f, 1.f);
    CHECK(lime.r() == 0.5f);
    CHECK(lime.g() == 1.0f);
    CHECK(lime.b() == 0.5f);
    CHECK(lime.a() == 1.f);

    Color purple;
    purple.setHSV(292.5f, 0.4f / 0.65f, 0.65f);
    CHECK(purple.r() == doctest::Approx(0.6f));
    CHECK(purple.g() == doctest::Approx(0.25f));
    CHECK(purple.b() == doctest::Approx(0.65f));
    CHECK(purple.a() == 1.f);

    Color white;
    white.setHSV(0.f, 0.f, 1.f);
    CHECK(white.r() == 1.f);
    CHECK(white.g() == 1.f);
    CHECK(white.b() == 1.f);
    CHECK(white.a() == 1.f);

    Color black;
    black.setHSV(0.f, 0.f, 0.f);
    CHECK(black.r() == 0.f);
    CHECK(black.g() == 0.f);
    CHECK(black.b() == 0.f);
    CHECK(black.a() == 1.f);
  }

  SUBCASE("Checking accessors") {
    Color color(1.f, 0.5f, 0.3f, 0.7f);
    CHECK(color[0] == 1.f);
    CHECK(color[1] == 0.5f);
    CHECK(color[2] == 0.3f);
    CHECK(color[3] == 0.7f);
    CHECK(color.vec4() == glm::vec4(1.f, 0.5f, 0.3f, 0.7f));
    CHECK(color.vec3() == glm::vec3(1.f, 0.5f, 0.3f));
  }

  SUBCASE("Checking color operations") {
    Color lime;
    lime.setHSV(120.f, 0.5f, 0.8f);

    auto inverted = lime.inverted();
    CHECK(inverted.h() == doctest::Approx(300.f));
    CHECK(inverted.s() == doctest::Approx(0.5f));
    CHECK(inverted.v() == doctest::Approx(0.2f));
    CHECK(inverted.inverted() == lime);

    auto complementary = lime.complementary();
    CHECK(complementary.h() == doctest::Approx(300.f));
    CHECK(complementary.s() == doctest::Approx(0.5f));
    CHECK(complementary.v() == doctest::Approx(0.8f));
    CHECK(complementary.complementary() == lime);

    Color white(1.f, 1.f, 1.f);
    Color black(0.f, 0.f, 0.f);

    CHECK(white.inverted() == black);
    CHECK(black.inverted() == white);
  }
}

} // namespace Illusion::Core
