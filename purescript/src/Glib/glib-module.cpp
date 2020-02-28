#include "purescript.h"
#include <glibmm/module.h>

using namespace purescript;

extern "C" auto PS_Glib_Module_open() -> const boxed& {
  static const boxed _ = [](const boxed& file_) -> boxed {
    return [=]() -> boxed { // effect
      const auto& file = unbox<string>( file_);
      auto m = std::make_shared<Glib::Module> ( file);
      if( ! *m) throw std::runtime_error( Glib::Module::get_last_error()); 
      return boxed( m);
    };
  };
  return _;
};

extern "C" auto PS_Glib_Module_symbol() -> const boxed& {
  static const boxed _ = [](const boxed& mod_) -> boxed {
    return [=](const boxed& sym__) -> boxed { // symbol
      return [=](const boxed& fret_) -> boxed {     // FuncRet
	return [=](const boxed&) -> boxed {   // drop proxy
	  return [=]() -> boxed {             // effect
	    const auto& mod = unbox< Glib::Module>( mod_);
	    const auto& sym_ = unbox< string>( sym__);
	    const auto& fret = unbox< dict_t>( fret_);
	    // get the c function name
	    std::string sym( "PS_" );
	    for (auto i = sym_.cbegin(), end = sym_.cend(); i != end; i++) {
	      auto c = *i;
	      if (c == '\'') sym.append( "Prime_");
	      else {
		if (c == '.') c = '_';
		sym.push_back( c);
	      }
	    }
	    void* s;
	    if( !mod.get_symbol( sym, s))
	      throw std::runtime_error( Glib::Module::get_last_error());
	    if( fret.contains( "Boxed")) {
	      typedef boxed (*PF_boxed)();
	      return reinterpret_cast<PF_boxed>( s)();
	    } else {
	      typedef const boxed& (*PF_crefBoxed)();
	      return reinterpret_cast<PF_crefBoxed>( s)();
	    }
	  };
	};
      };
    };
  };
  return _;
};
