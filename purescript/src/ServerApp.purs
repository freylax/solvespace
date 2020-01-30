module ServerApp where

import Prelude (Unit)
import Effect ( Effect)
import Server( server)
import Network.GioTcpServer (Server)

main :: Server -> Effect Unit
main = server
