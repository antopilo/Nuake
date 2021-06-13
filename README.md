# Nuake


# What is it
Nuake is a game engine written from scratch by myself. It is not meant to be a end-all be-all engine and it is not a quakespasm type engine. This is a game engine that focuses on fast level design iteration that integrates with quake level editing software. If you can create quake maps, you can create Nuake levels. 
`Warning: It is still very early in development and I dont recommend anyone using this to make their games *yet*. `

# How to build
1. Clone the repos using `git clone --recurse-submodules https://github.com/antopilo/Nuake.git`
2. Run the `generate.bat` to generate the sln files.
3. Open `Nuake.sln`
4. Build and run

# Contributing
Feel free to make pull requests and I will look over them myself.

# Documentation
You can access the current documentation at [here](https://nuake.readthedocs.io/en/latest/index.html)

# Features
- Trenchbroom integration with live reload
- fast ECS & Scene tree system
- Bullet physics
- PBR rendering
- Volumetric lighting
- Parallax mapping
- Game in editor
- Procedural & HDR skies
- Image based lighting
- Modern UI system
- Wren scripting api and module system

# Planned
- Advanced bullet physics features
  - Soft bodies, Joints, etc.
- Demo level
- Cross platform
- Different rendering API(not a priority)
- Triggers and entity editing in TB
- Optimization
- Water simulation
- Terrain editing
- Exporting
- Custom shaders
