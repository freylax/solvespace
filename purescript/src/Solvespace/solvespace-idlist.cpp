#include "purescript.h"
#include "solvespace.h"
#include "dsc.h"
#include "sketch.h"
#include <iostream>

namespace {
  using namespace purescript;
  template <typename T, typename H>
  static auto idListFoldrImpl( const boxed& f) -> boxed {
    return [=](const boxed& init) -> boxed {
      return [=](const boxed& xs_) -> boxed {
	auto acc = init;
	auto xs = unbox< IdList<T,H>* >(xs_);
	if(!xs->IsEmpty()) {
	  T* it = xs->end()-1; T* end = xs->begin()-1;
	  for (;it != end; it--) {
	    acc = f( box<T*>(it))(acc);
	    //acc = f(std::shared_ptr<T>(it,[](T*){}))(acc);
	  }
	}
	return acc;
      };
    };
  };

  template <typename T, typename H>
  static auto idListFoldlImpl( const boxed& f) -> boxed {
    return [=](const boxed& init) -> boxed {
      return [=](const boxed& xs_) -> boxed {
	auto acc = init;
	auto xs = unbox< IdList<T,H>* >(xs_);
	T* it = xs->begin(); T* end = xs->end();
	for(; it != end; it++) {
	  acc = f(acc)(box<T*>(it));
	  //acc = f(acc)(std::shared_ptr<T>(it,[](T*){}));
	}
	return acc;
      };
    };
  };
}
   
using namespace purescript;


#define CREATE_INSTANCES_FOR(TYPE)					\
  extern "C" auto PS_Solvespace_IdList_foldr##TYPE##List() -> const boxed& { \
    static const boxed _ = idListFoldrImpl<TYPE,h##TYPE>;		\
    return _;								\
  };									\
  extern "C" auto PS_Solvespace_IdList_foldl##TYPE##List () -> const boxed& { \
    static const boxed _ = idListFoldlImpl<TYPE,h##TYPE>;		\
    return _;								\
  };									\


CREATE_INSTANCES_FOR( Entity )

extern "C" auto PS_Solvespace_IdList_showEntityImpl() -> const boxed& {
  static const boxed _ = [=](const boxed& e_) -> boxed {
    auto e = unbox<Entity*>( e_);
    return boxed( std::to_string( static_cast<uint32_t>( e->type )));
  };
  return _;
};

extern "C" auto PS_Solvespace_IdList_entityListImpl() -> const boxed& {
  static const boxed _ = [=]( const boxed& sketch_) -> boxed {
    return [=]() -> boxed { // effect
      auto sketch = unbox<Sketch*>( sketch_);
      typedef IdList<Entity,hEntity> L;
      return box<L*>(static_cast<L*>(&sketch->entity));
      // return std::shared_ptr<L>
      // 	( static_cast<L*>(&sketch->entity)
      // 	  , [](L*){});
    };
  };
  return _;
}


extern "C" auto PS_Solvespace_IdList_entityListN() -> const boxed& {
  static const boxed _ = [=]( const boxed& l_) -> boxed {
    auto l = unbox<IdList<Entity,hEntity>*>( l_);
    return boxed( l->n);
  };
  return _;
};
 
