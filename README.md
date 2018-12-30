<p align="center"> 
  <img src ="doc/logo.svg" />
</p>

In order to learn the concepts behind Vulkan, I am creating Illusion. For now, Illusion is a convenience layer on top of Vulkan, similar in spirit to [V-EZ](https://github.com/GPUOpen-LibrariesAndSDKs/V-EZ). However, I plan to add more features as I progress in learning.

### Features

- [x] A ring-buffer for per-frame resources
- [x] Automatic creation of pipeline layouts using glslang and spirv-cross
- [x] Automatic allocation and updates of descriptor sets
- [x] Loading of GLTF models (no support formorph targets yet)
- [x] Conversion of equirectangular panoramas to cubemaps
- [x] Creation of prefiltered irradiance and reflectance maps for physically based shading
- [ ] Automatic image layout transitions
- [ ] Parallel command buffer recording
- [ ] A frame or render-graph to automatically create renderpasses and subpasses with dependencies
- [ ] ...

### Dependencies

Most dependencies are included as [git submodules](externals). Additionally you will need a C++ compiler, CMake and the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/).

### Building Illusion

##### Linux

```bash
git clone https://github.com/Simmesimme/illusion.git
cd illusion
git submodule update --init
mkdir build && build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install ..
make -j install
```

### Included Examples

Link | Description | Screenshot
-----|-------------|-----------
[Triangle](examples/triangle) | A simple triangle without vertex or index buffers that uses a pre-recorded command buffer and no per-frame resources. | pic

### MIT License

Copyright (c) 2018 Simon Schneegans

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
