#pragma once
#include "GameBaseObject.h"
#include <vector>
#include "GameEntity.h"
#include "../EngineMain.h"

class GameComponent : public GameBaseObject
{
public:
	virtual void OnSpawn() override;
	virtual void OnDestroy() override;
	virtual void Destroy() final override { GameBaseObject::Destroy(); }

	void SetParentEntity(GameEntity* Entity);
	GameEntity* GetParentEntity();

private:
	GameEntity* ParentEntity = nullptr;
};

template <class T>
T* CreateComponent(GameEntity* Entity)
{
	GameBaseObject::bCreationLock = false;
	T* Object = new T();

	Check(dynamic_cast<GameComponent*>(Object) != nullptr);

	dynamic_cast<GameComponent*>(Object)->SetParentEntity(Entity);

	GameBaseObject::bCreationLock = true;
	return Object;
};
