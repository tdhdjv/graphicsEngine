#ifndef GL_ERROR_HEADER 
#define GL_ERROR_HEADER
#include <glad/glad.h>

char* errorToString(GLenum errorStatus) {
  switch(errorStatus){
    case GL_INVALID_ENUM:
      return "INVALID ENUM";
      break;
    case GL_INVALID_VALUE:
      return "INVALID VALUE";
      break;
    case GL_INVALID_OPERATION:
      return "INVALID OPERATION";
      break;
    case GL_STACK_OVERFLOW:
      return "STACK OVERFLOW";
      break;
    case GL_STACK_UNDERFLOW:
      return "STACK UNDERFLOW";
      break;
    case GL_OUT_OF_MEMORY:
      return "OUT OF MEMORY";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "INVALID FRAMEBUFFER OPERATION";
      break;
    case GL_CONTEXT_LOST:
      return "CONTEXT LOST";
      break;
    default:
      return "UNKNOWN ERROR";
      break;
  }
}

#define GL_CHECK(code) code;
/*
  while(glGetError() != GL_NO_ERROR);\
  code;\
  for(GLenum errorStatus = glGetError(); errorStatus != GL_NO_ERROR; errorStatus = glGetError()) {\
    printf("Error %s: at file: %s, line: %d", errorToString(errorStatus), __FILE__, __LINE__);\
  }
*/

#endif
