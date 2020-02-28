module PluginLoader
       ( pluginLoader )   
       where

import Prelude (Unit, bind, discard, pure, unit, ($), (<>))
import Effect ( Effect)
import Effect.Exception ( try)
import Data.Show (show)
import Data.String.Pattern (Pattern(..))
import Data.String.Common (split, trim)
import Type.Proxy (Proxy(..))
import Glib.Module (open,symbol,Module,FuncRet(..))
import Network.GioTcpServer (Server,Socket, listen, onRead, read, write)
import Effect.Ref as Ref 
import Effect.Class (liftEffect)
import Data.Maybe  (Maybe(..))
import Data.Either (Either(..))

-- _EffectFct :: forall a. Proxy (a -> Effect String)
_EffectFct :: Proxy (Effect String)
_EffectFct = Proxy

--serve :: forall a. a -> Ref.Ref (Maybe DLObjects) -> Socket -> Effect Unit
serve :: Ref.Ref (Maybe Module) -> Socket -> Effect Unit
serve objs socket = do
  s <- read socket
  r <- interp $ split (Pattern " ") $ trim s
  write socket $ r <> "\n"
  pure unit
  where
    interp :: Array String -> Effect String
    interp ["l", lib ] = do
      res <- try $ open $ lib <> ".so"
      case res of
        Right mod -> do
          _ <- Ref.write (Just mod) objs
          pure $ "loaded " <> lib <> ".so"
        Left error -> pure $ show error
    interp ["c", ret, fct] = do
      mod_ <- Ref.read objs
      case mod_ of
        Just mod -> do
          res <- try $ symbol mod fct (interpRet ret) _EffectFct
          case res of
            Right f -> do
              res' <- try $ f 
              case res' of
                Left error -> pure $ "error:" <> show error
                Right s -> pure s
            Left error -> pure $ show error
        Nothing -> pure "no object file loaded"
        where
          interpRet :: String -> FuncRet
          interpRet "b" = Boxed
          interpRet _ = ConstRefBoxed
    interp _ = pure $ "l <file(.so)>  : load plugin\n"
                   <> "c b|r <Module.fct> : call function, returns <b>oxed or const <r>ef boxed"

pluginLoader ::  Server -> Effect Unit
pluginLoader srv = do
--  srv <- createServer port
  dlobj <- liftEffect (Ref.new Nothing)
  onRead srv $ serve dlobj
  listen srv
  pure unit              

