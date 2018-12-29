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

Here are the instructions for Linux.

```bash
git clone https://github.com/Simmesimme/illusion.git
cd illusion
git submodule update --init
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DNSTALL_PREFIX=install ..
make -j install
```
