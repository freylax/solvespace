module Plugin where

import Prelude ( bind, pure, show, ($)) 
import Effect ( Effect)
-- import Effect.Console (log)
--import Effect.Ref as Ref

--add :: Ref.Ref Int -> Effect String
--add i' = do
--  i <- Ref.read i'
--  _ <- Ref.write (i + 1) i'
--  pure $ (show i) <> " -> " <> (show $ i + 1)
  
  
import Solvespace.IdList (entityList,entityListN)
import Solvespace.Resources (testSketch) 

showEntities :: Effect String
showEntities = do
  xs <- entityList
  pure $ show xs

showEntitiesN :: Effect String
showEntitiesN = do
  xs <- entityList
  pure $ show (entityListN xs)
  

test :: Effect String
test = testSketch
       
test2 :: Effect String
test2 = pure "Hello"
