module Server where

import Prelude (Unit, discard, pure, unit, ($), (<>)) -- bind
import Effect ( Effect)
import Effect.Console ( log)
import Effect.Exception ( Error, message, catchException)
--import Effect.Ref as Ref 
--import Effect.Class (liftEffect)
import Network.GioTcpServer (Server)
import PluginLoader (pluginLoader)
import Solvespace.Resources (Sketch, setSketch)


printError :: Error -> Effect Unit
printError e = do
  log $ "Error:" <> message e
  pure unit

server :: Sketch -> Server -> Effect Unit
server sketch srv = catchException printError
   do
     pure $ setSketch sketch
     --i <- liftEffect (Ref.new 0)
     pluginLoader srv 
     pure unit              
