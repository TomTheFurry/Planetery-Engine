# Planetery-Engine
A C++ game engine written using basic api (PhysX, Vulkan...) which aims to be used in a future game(s) about space
TODO: Write better description here

<h1>Documentation</h1>
<details>
<h3>Vulkan Interface</h3>
<details>
<h4>Sub Modules</h4>
<details>
<p>

* Dclaration
* Enum
* Device
* Swapchain (WORK IN PROGESS)
* Extension (TODO)
* Sync
* Buffer
* Image
* Pipeline
* Descriptor
* Tick (Will be Deplicated)
* Memory
* Lifetime
* Commend

</p>
</details>

<h4>Sub Module: Declaration</h4>
<details>
A list of class and struct declaration. Also declares the base object class:
class ComplexObject
</details>

<h4>Sub Module: Enum</h4>
<details>
All enum is in here. Needs reorganizing...
</details>

<h4>Sub Module: Device</h4>
<details>
<p>
Class list:

```
class PhysicalDevice {
//...
}
class LogicalDevice {
//...      
}
```

</p>
</details>

<h4>Sub Module: Swapchain</h4>
<details>
TODO
</details>

<h4>Sub Module: Extension</h4>
<details>
TODO
</details>

<h4>Sub Module: Sync</h4>
<details>
<p>
Class list:

```
class Semaphore {
//...
}
class TimelineSemaphore {
//...      
}
class Fence {
//...      
}
```

</p>
</details>

<h4>Sub Module: Buffer</h4>
<details>
<p>
Class list:

```
class Buffer {
//...
}
class StorageBuffer {
//...      
}
class UniformBuffer {
//...      
}
class StagingBuffer {
//...      
}
class VertexBuffer {
//...      
}
class IndiceBuffer {
//...      
}
```

</p>
</details>

<h4>Sub Module: Image</h4>
<details>
<p>
Class list:

```
class Image {
//...
}
class ImageView {
//...      
}
class FrameBuffer {
//...      
}
```

</p>
</details>

<h4>Sub Module: Pipeline</h4>
<details>
<p>
Class list:

```
class RenderPass {
class Subpass {}
//...
}
class Pipeline {
//...
}
```

</p>
</details>

<h4>Sub Module: Descriptor</h4>
<details>
<p>
Class list:

```
class DescriptorLayout {
//...
}
class DescriptorPool {
//...
}
class DescriptorSet {
//...
}
```

</p>
</details>

<h4>Sub Module: Memory</h4>
<details>
<p>
Class list:

```
class MemoryAllocator {
//...
}
class MemoryPool {
//...
}
```

</p>
</details>

<h4>Sub Module: Lifetime</h4>
<details>
<p>
Class list:

```
class LifetimeManager {
//...
}
class MonotonicLifetimeManager {
//...
}
```

</p>
</details>

<h4>Sub Module: Commend</h4>
<details>
<p>
Class list:

```
class CommendPool {
//...
}
class CommendBuffer {
//...
}
```

</p>
</details>

</details>
</details>
