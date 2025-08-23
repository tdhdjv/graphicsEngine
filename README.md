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
- [ ] code refactoring
- [ ] ambient lighting (diffuse)
- [ ] post processes
- [ ] model matrix loading
- [ ] imgui
- [ ] multiple light
- [ ] animations
- [ ] improve integers type namings
