module Solvespace.IdList where

import Prelude ((<>))
import Data.Foldable (class Foldable, foldMapDefaultR, foldl) --(Foldable, foldMapDefaultR)
import Data.Show (class Show, show )
import Effect (Effect)
import Solvespace.Resources (Sketch, getSketch)

foreign import data EntityList :: Type -> Type
foreign import data Entity :: Type

foreign import foldrEntityList :: forall  a b. (a -> b -> b) -> b -> EntityList a -> b
foreign import foldlEntityList :: forall  a b. (b -> a -> b) -> b -> EntityList a -> b

instance foldableEntityList :: Foldable EntityList where
   foldr = foldrEntityList
   foldl = foldlEntityList
   foldMap = foldMapDefaultR

-- foreign import foldrIdList :: forall a b. (a -> b -> b) -> b -> IdList a -> b
-- foreign import foldlIdList :: forall a b. (b -> a -> b) -> b -> IdList a -> b
-- foreign import data EntityList :: Type

-- instance foldableIdList :: Foldable EntityList where
--   foldr = foldrEntityList
--   foldl = foldlEntityList
--   foldMap = foldMapDefaultR


-- foldrIdList :: forall b. (Entity -> b -> b) -> b -> IdList Entity -> b
-- foldrIdList = foldrIdListEntity
  
-- foreign import foldrIdListEntity :: forall b. (Entity -> b -> b) -> b -> IdList Entity -> b
-- foreign import foldlIdListEntity :: forall b. (b -> Entity -> b) -> b -> IdList Entity -> b

--newtype EntityList = IdList Entity

instance showEntityList :: Show (EntityList Entity) where
  show = foldl acc "Entities:"
    where
      acc :: String -> Entity -> String
      acc s e = s <> " " <> ( show e )

instance showEntity :: Show Entity where
  show = showEntityImpl

foreign import showEntityImpl :: Entity -> String

entityList :: Effect (EntityList Entity) 
entityList = entityListImpl getSketch

foreign import entityListImpl :: Sketch -> Effect (EntityList Entity) 

foreign import entityListN :: (EntityList Entity) -> Int

                              
