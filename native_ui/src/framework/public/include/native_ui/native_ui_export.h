#pragma once

#if defined(_WIN32)
  #if defined(NATIVE_UI_SHARED_LIBRARY)
    #define NATIVE_UI_API __declspec(dllexport)
  #else
    #define NATIVE_UI_API __declspec(dllimport)
  #endif
#else
  #if defined(NATIVE_UI_SHARED_LIBRARY)
    #define NATIVE_UI_API __attribute__((visibility("default")))
  #else
    #define NATIVE_UI_API
  #endif
#endif
