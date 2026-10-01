# vkFFT for fast fourier transform.

vkFFT leverages vulkan on linux

```aiignore
sudo apt-get update && sudo apt-get install -y \
    libvulkan-dev \
    vulkan-tools
```
also need:

```aiignore
sudo apt install glslang-dev glslang-tools glslc
```

# MacOS

Brew can be used to install the dependencies.

```aiignore
brew install vulkan-headers
brew install molten-vk
brew install vulkan-loader
brew install glslang
```

The include path for glslang is a bit different on macOS
```aiignore
/opt/homebrew/include/glslang/Include
```

# vkFFT Example:

The git repo for vkFFT contains an example usage:

https://github.com/DTolm/VkFFT/blob/master/VkFFT_TestSuite.cpp

API Guide is in the src directory in the _deps folder.

There is also an IEEE article:

https://ieeexplore.ieee.org/document/10036080/


