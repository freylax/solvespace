module Server where

import Prelude (Unit, bind, discard, pure, unit, ($), (<>))
import Effect ( Effect)
import Effect.Console ( log)
import Effect.Exception ( Error, message, catchException)
import Effect.Ref as Ref 
import Effect.Class (liftEffect)
import Network.GioTcpServer (Server)
import PluginLoader (pluginLoader)

printError :: Error -> Effect Unit
printError e = do
  log $ "Error: " <> message e
  pure unit

server :: Server -> Effect Unit
server srv = catchException printError
   do
     i <- liftEffect (Ref.new 0)
     pluginLoader srv i
     pure unit              
