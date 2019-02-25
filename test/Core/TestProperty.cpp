////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/Property.hpp>

#include <doctest.h>

namespace Illusion::Core {

TEST_CASE("Illusion::Core::Property") {
  Double pDouble;
  Float  pFloat;
  Int8   pInt8;
  Int16  pInt16;
  Int32  pInt32;
  Int64  pInt64;
  UInt8  pUInt8;
  UInt16 pUInt16;
  UInt32 pUInt32;
  UInt64 pUInt64;
  Bool   pBool;
  String pString;
  FVec2  pFVec2;
  FVec3  pFVec3;
  FVec4  pFVec4;
  DVec2  pDVec2;
  DVec3  pDVec3;
  DVec4  pDVec4;
  IVec2  pIVec2;
  IVec3  pIVec3;
  IVec4  pIVec4;
  UVec2  pUVec2;
  UVec3  pUVec3;
  UVec4  pUVec4;
  FMat3  pFMat3;
  FMat4  pFMat4;
  DMat3  pDMat3;
  DMat4  pDMat4;

  SUBCASE("Checking default constructors") {
    CHECK(pDouble.get() == 0.0);
    CHECK(pFloat.get() == 0.f);
    CHECK(pInt8.get() == 0);
    CHECK(pInt16.get() == 0);
    CHECK(pInt32.get() == 0);
    CHECK(pInt64.get() == 0);
    CHECK(pUInt8.get() == 0u);
    CHECK(pUInt16.get() == 0u);
    CHECK(pUInt32.get() == 0u);
    CHECK(pUInt64.get() == 0u);
    CHECK(pBool.get() == false);
    CHECK(pString.get() == "");
    CHECK(pFVec2.get() == glm::fvec2());
    CHECK(pFVec3.get() == glm::fvec3());
    CHECK(pFVec4.get() == glm::fvec4());
    CHECK(pDVec2.get() == glm::dvec2());
    CHECK(pDVec3.get() == glm::dvec3());
    CHECK(pDVec4.get() == glm::dvec4());
    CHECK(pIVec2.get() == glm::ivec2());
    CHECK(pIVec3.get() == glm::ivec3());
    CHECK(pIVec4.get() == glm::ivec4());
    CHECK(pUVec2.get() == glm::uvec2());
    CHECK(pUVec3.get() == glm::uvec3());
    CHECK(pUVec4.get() == glm::uvec4());
    CHECK(pFMat3.get() == glm::fmat3());
    CHECK(pFMat4.get() == glm::fmat4());
    CHECK(pDMat3.get() == glm::dmat3());
    CHECK(pDMat4.get() == glm::dmat4());
  }
}

} // namespace Illusion::Core
