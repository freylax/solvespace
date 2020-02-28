#include "purescript.h"
#include "solvespace.h"
#include <string>
#include <iostream>
namespace
{
  using namespace purescript;
  static boxed sketch = box( 0);
}

extern "C" auto PS_Solvespace_Resources_setSketch() -> const boxed& {
  static const boxed _ = [=](const boxed& s) -> boxed {
    std::cout << "setSketch" << std::endl;
    sketch = s;
    return boxed();
  };
  return _; 
};

extern "C" auto PS_Solvespace_Resources_getSketch() -> const boxed& {
  static const boxed _ = []() -> boxed {
    return sketch;
  };
  return _; 
};

extern "C" auto PS_Solvespace_Resources_testSketch() -> const boxed& {
  static const boxed _ = []() -> boxed {
    Sketch* s = unbox< Sketch*>( sketch);
    return std::to_string( s->entity.n );
  };
  return _; 
};
