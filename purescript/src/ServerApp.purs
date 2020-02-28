module ServerApp where

import Prelude (Unit)
import Effect ( Effect)
import Server( server)
import Network.GioTcpServer (Server,createServer)
import Solvespace.Resources (Sketch)

-- this is a dummy to get dependencies build
createServer_ :: Int -> Effect Server
createServer_ = createServer 

main :: Sketch -> Server -> Effect Unit
main = server  
