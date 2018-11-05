////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/FPSCounter.hpp>
#include <Illusion/Core/Logger.hpp>

#include <GLFW/glfw3.h>

#include <stb_image.h>
#include <gli/gli.hpp>
#include <glm/glm.hpp>

#include <iostream>

int main(int argc, char* argv[]) {
#ifdef NDEBUG
    Illusion::Core::Logger::enableDebug = false;
    Illusion::Core::Logger::enableTrace = false;
    // auto engine = std::make_shared<Illusion::Graphics::Engine>("SimpleWindow", false);
#else
    Illusion::Core::Logger::enableDebug = true;
    Illusion::Core::Logger::enableTrace = true;
    // auto engine = std::make_shared<Illusion::Graphics::Engine>("SimpleWindow", true);
    // engine->getPhysicalDevice()->printInfo();
#endif

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    gli::texture texture = gli::load("fileName");

  ILLUSION_MESSAGE << "FPS: " << glm::clamp(1, 0, 2) << std::endl;

  return 0;
}
