Rules for better type naming (the type should tell you what it is being used for):
    Always use the string I made!!! (unless it is an external api that uses c strings)
    Use the GLuint
    Use size_t when handling pointers/size (since they are dependent on the machine)
    else use <stdint> type never int, char ... types (expection: char as actually ascii characters)
    

Goal Try to finish 1 per day

- [x] obj model loading
- [x] textures
- [x] scene managment
- [x] window resizing
- [x] improve scene managment
- [x] make a gltf parser
- [x] material handling
- [x] environment (skybox)
- [x] ambient lighting (specular)
- [x] code refactoring
- [x] ambient lighting (diffuse)
- [x] make the code more modular and also add some generality?
- [x] fully clean up the code
- [x] post processes
- [x] optimize post processing
- [x] improve model loading
- [x] fix bloom offset
- [x] multiple light
- [x] model matrix loading
- [ ] refactor again, fuck!
- [ ] deferred shading
- [ ] ssao
- [ ] optimize the binding
- [ ] batch rendering
- [ ] animations

# Pre Draw

1. instancing

# Pipeline design
1. Pre Render
2. Render
3. Post Process

## Basic Idea

Scene: Have a scene be the description of the scene
RenderData: One Draw Call
RenderBuffer: An Array of RenderData

Scene:
    Meshes (The geometry of the thing we want)
    Objects (reference to mesh or a parent of another objects + transform)
    Camera
    Skybox

## Pre Render

1. frustum culling
1. sort by material

## Render

1. render pass
2. deffer pass

## Post Process

1. ssao
1. bloom
1. tonemap

