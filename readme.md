# Boat-Water Interaction

A real-time boat-and-water physics + rendering simulation from scratch — implementing realistic water surface, buoyancy, drag, and boat motion using mesh-based water and physics, without relying on existing water-plugins.  

Check out the project media: https://www.shauryapandey.tech/portfolio/boat-water-interaction

## 🚤 What this project is / Why it exists

When I started this, I wanted to go beyond using pre-made tools and challenge myself: build water and boat interaction from first principles.  
The result is a system that simulates realistic water surfaces (using Gerstner wave equations) and boat dynamics — handling submerged-triangle detection, buoyancy, drag & viscosity, and stable interaction between water surface and boat hull.  

It’s a demonstration of how to combine mathematics, physics, real-time rendering and performance optimization to approximate natural water behavior in a game/graphics environment.  

## 🧩 Features / What it does

- Procedural water surface generation using wave equations (Gerstner waves) for realistic ocean motion.  
- Real-time deformation of mesh vertices (including upright and horizontal displacement) to simulate flowing waves.  
- Correct lighting via real-time normal calculation for dynamically deforming meshes.  
- Physics-based boat–water interaction: submerged-triangle detection, buoyant force computation, drag & viscosity, accounting for partial immersion.  
- Performance optimizations: GPU-based rendering of waves; multi-threaded force/physics computations.  
- Modular and maintainable architecture: separation between rendering layer and physics logic, designed for clarity, testability, and extensibility.  

## 📂 High-level Architecture / How it’s built

- Ocean rendered with a procedural mesh updated each frame using a material shader (on GPU) to compute wave deformation.  
- Physics logic (submersion detection, buoyancy, drag, force application) computed in native code (or core logic module), independent of rendering engine specifics — this separation improves portability and testability.  
- Multi-threaded physics tasks: force computations batched across mesh elements to balance parallelization overhead and workload, improving performance when many triangles are submerged.  
- Debugging tools: visual debug-lines and projections to verify hull-water intersection and submerged-triangle detection before moving to complex meshes.  

## 📥 Try it out / Demo

There is a playable demo of this project on itch.io:  
[Play the Boat-Water Interaction Simulation](https://shauryapandey.itch.io)  

## 🌱 What I learned / Challenges & Solutions

- Handling mesh deformation + lighting: computing normals dynamically after mesh deformation to ensure correct real-time lighting.  
- Submerged-triangle detection and partial immersion: early brute-force triangle checks proved inefficient; switching to sampling wave height via wave equations at boat-vertex positions made it more accurate and efficient.  
- Physics stability requires careful unit and scale management — e.g. converting to correct units (centimeters vs meters), matching force/weight units.  
- Performance optimization: moving heavy tasks (wave rendering) to GPU; batching computations and multi-threading physics to maintain stable frame rates even under heavy load.  
- Project architecture: separating physics from rendering makes code more maintainable and easier to refactor — especially when migrating to more complex boat meshes or different rendering engines.  

## 💡 Why this project matters / Use-cases

- As a learning experiment: it shows how realistic water + buoyancy + physics can be simulated from first principles, which can help when building custom game engines or graphics simulations.  
- For games or simulations needing realistic water-boat interaction without relying on engine-specific plugins — giving full control over physics, performance, and behavior.  
- As a technical demonstration of combining mathematics, real-time rendering, physics, and performance engineering in a single project.  

## 🧠 Possible Future Improvements

- Extend support to arbitrary complex boat meshes (beyond the proxy cube/hull) and ensure stable physics for complex geometries.  
- Add more realistic water effects: waves with variable wind, wave-breaking, foam, spray, underwater effects.  
- Improve performance further — especially for large bodies of water + many interacting objects.  
- Add more advanced drag/viscosity / fluid-dynamics approximations (or integrate simplified fluid solvers) for more realism.  
- Add a configurable setup for different water and boat parameters (density, viscosity, boat mass, hull shape, etc.) to make system more generic.  


Project by **Shaurya Pandey** — building from first principles; concept, implementation, and demo.  
