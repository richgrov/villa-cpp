# vkad

A simple, minimal-dependency CAD program powered by Vulkan

# Setup

Download the [Vulkan SDK](https://vulkan.lunarg.com/)

Compile shaders:

```
glslc src/shader/shader.vert -o shader-vert.spv
glslc src/shader/shader.frag -o shader-frag.spv
```

Build & run:
```
mkdir build
cmake -B build
cmake --build build
./build/example/example(.exe)
```

# Credits

Font by [zedseven](https://github.com/zedseven/Pixellari)

Background by [Mockingjay1701](https://www.deviantart.com/mockingjay1701/art/Pixel-art-landscape-525082296)
