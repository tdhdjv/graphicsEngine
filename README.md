Rules for better type naming (the type should tell you what it is being used for):
    Always use the string I made!!! (unless it is an external api that uses c strings)
    Use the GLuint
    Use size_t when handling pointers/size (since they are dependent on the machine)
    else use <stdint> type never int, char ... types (expection: char as actually ascii characters)
    

- [x] obj model loading
- [x] textures
- [x] scene managment
- [x] window resizing
- [ ] improve scene managment
- [ ] make a gltf parser
- [ ] improve integers type namings
