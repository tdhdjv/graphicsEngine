Rules for better type naming (the type should tell you what it is being used for):
    Always use the string I made!!! (unless it is an external api that uses c strings)
    expections filenames because it is tedious otherwise
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
- [ ] post processes
- [ ] model matrix loading
- [ ] multiple light
- [ ] animations
- [ ] improve integers type namings
