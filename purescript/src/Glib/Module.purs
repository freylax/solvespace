module Glib.Module 
       ( Module
       , open
       , symbol
       , FuncRet(..)
       )
       where

import Effect ( Effect)
import Type.Proxy ( Proxy)

data FuncRet = ConstRefBoxed | Boxed 

foreign import data Module :: Type
                    
foreign import open :: String -> Effect Module

foreign import symbol ::  forall a . Module -> String -> FuncRet -> Proxy a -> Effect a

