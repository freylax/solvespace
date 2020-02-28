module Solvespace.Resources where
import Data.Unit ( Unit)
import Effect ( Effect)

foreign import data Sketch :: Type
foreign import setSketch :: Sketch -> Unit
foreign import getSketch :: Sketch
foreign import testSketch :: Effect String
