////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/Logger.hpp>
#include <Illusion/Core/Property.hpp>

#include <iostream>

////////////////////////////////////////////////////////////////////////////////////////////////////
// The Signals and Properties of Illusion use pure C++11 and allow for automatic notifications    //
// whenever a value changed. As you can imagine, it’s possible to do a lot of things here. There  //
// are numerous applications of these patterns and I really like the readability of the code. If  //
// you put these properties as members into classes the communication design becomes much easier  //
// and the class interface will be more intuitive.                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {

  // In this simple example we will connect an output value to an input value and emit a signal when
  // the output value exceeds a given threshold.
  Illusion::Core::Property<float>     input;
  Illusion::Core::Property<float>     output;
  Illusion::Core::Signal<std::string> signal;

  // Whenever the input value is changed, the output value will be changed as well.
  output.connectFrom(input);

  // Whenever the output value changes, we print the value. When the value is larger than 0.5 we
  // will emit the signal.
  output.onChange().connect([&signal](float val) {
    Illusion::Core::Logger::message() << "Output: " << val << std::endl;
    if (val > 0.5f) {
      signal.emit("Danger!");
    }

    // If we return false here, the onChange handler will be disconnected.
    return true;
  });

  // Connect a lambda to the signal. Of course you can connect as many methods as you want. Once the
  // signal is emitted, the connected slots will be called in the order they have been connected in.
  // You can also connect class member methods. The connect() method actually returns a connection
  // ID which can be used to disconnect a slot again.
  signal.connect([](std::string const& message) {
    // When the signal is emitted, we will print the message.
    Illusion::Core::Logger::message() << "Critical situation: " << message << std::endl;

    // Stay connected.
    return true;
  });

  // The output of the following three lines will be:
  // [ILLUSION][M] Output: 0.2
  // [ILLUSION][M] Output: 0.4
  // [ILLUSION][M] Output: 0.6
  // [ILLUSION][M] Critical situation: Danger!
  input = 0.2;
  input = 0.4;
  input = 0.6;

  return 0;
}
