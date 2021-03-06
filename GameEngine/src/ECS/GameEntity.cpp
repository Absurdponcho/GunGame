#include "GameEntity.h"
#include "../Logging/Logging.h"
#include "GameComponent.h"
#include <glm/gtx/euler_angles.hpp>
#include "../GameManager.h"
#include "../Physics/RigidbodyComponent.h"

void GameEntity::OnTick(float DeltaTime)
{
	GameBaseObject::OnTick(DeltaTime);
}
void GameEntity::Destroy()
{
	for (int Index = (int)AttachedComponents.size() - 1; Index >= 0; Index--)
	{
		if (!AttachedComponents[Index]->IsPendingDestroy())
		{
			AttachedComponents[Index]->Destroy();
		}
	}
	AttachedComponents.clear();
	GameBaseObject::Destroy();

}

glm::mat4x4 EntityTransform::GetModelMatrix()
{
	if (bModelMatrixDirty)
	{
		glm::mat4x4 TranslateMatrix;
		if (GameEntity* Entity = AttachedEntity.Get())
		{
			TranslateMatrix = glm::translate(Entity->GetTransform().GetModelMatrix(), Position);
		}
		else
		{
			TranslateMatrix = glm::translate(glm::mat4x4(1.0f), Position);
		}
		glm::mat4x4 RotateMatrix = TranslateMatrix * glm::toMat4(Rotation);
		glm::mat4x4 ScaleMatrix = glm::scale(RotateMatrix, Scale);
		CachedModelMatrix = ScaleMatrix;
		bModelMatrixDirty = false;
	}
	return CachedModelMatrix;
}

void EntityTransform::SetDirtyMatrixCache()
{
	bModelMatrixDirty = true;
	bTransRotationMatrixDirty = true;

	for (GameWeakObjectPointer<GameEntity> Entity : Children)
	{
		if (GameEntity* Child = Entity.Get())
		{
			Child->GetTransform().SetDirtyMatrixCache();
		}
	}
}

glm::mat4x4 EntityTransform::GetTransRotationMatrix()
{
	if (bTransRotationMatrixDirty)
	{
		glm::mat4x4 TranslateMatrix;
		if (GameEntity* Entity = AttachedEntity.Get())
		{
			TranslateMatrix = glm::translate(Entity->GetTransform().GetTransRotationMatrix(), Position);
		}
		else
		{
			TranslateMatrix = glm::translate(glm::mat4x4(1.0f), Position);
		}
		glm::mat4x4 RotateMatrix = TranslateMatrix * glm::toMat4(Rotation);
		CachedTransRotationMatrix = RotateMatrix;
		bTransRotationMatrixDirty = false;
	}
	return CachedTransRotationMatrix;
}

void EntityTransform::AttachTo(GameEntity* Entity)
{
	AttachedEntity = GameWeakObjectPointer<GameEntity>(Entity);
	if (Entity)
	{
		Entity->GetTransform().AddChild(GameWeakObjectPointer<GameEntity>(Parent));
	}
}

void EntityTransform::AddChild(GameWeakObjectPointer<GameEntity> Child)
{
	Children.push_back(Child);
}

EntityTransform& GameEntity::GetTransform()
{
	return Transform;
}

void GameEntity::AttachComponent(GameComponent* Component)
{
	Check(Component);
	Check(!GameBaseObject::bCreationLock);

	AttachedComponents.push_back(Component);

	LOGVERBOSE("GameEntity::AttachComponent()", "Component (" + std::to_string(Component->GetUID()) + ") attached to entity (" + std::to_string(GetUID()) + ")");
}

void GameEntity::DetachComponent(GameComponent* Component)
{

	Check(Component);
	if (Component == nullptr) return;
	for (int Index = 0; Index < AttachedComponents.size(); Index++)
	{
		if (Component->GetUID() == AttachedComponents[Index]->GetUID())
		{
			AttachedComponents.erase(AttachedComponents.begin() + Index);
			return;
		}
	}
}

void EntityTransform::SetEulerRotation(glm::vec3 Euler)
{
	Euler *= 0.0174533f; // deg to rad
	Rotation = glm::quat(Euler);

	//bModelMatrixDirty = true;
	//bXMirroredModelMatrixDirty = true;
	//bTransRotationMatrixDirty = true;
	SetDirtyMatrixCache();
}

glm::vec3 EntityTransform::GetEulerRotation()
{
	return glm::eulerAngles(GetRotation()) / 0.0174533f;
}

glm::vec3 EntityTransform::GetWorldEulerRotation()
{
	return glm::eulerAngles(GetWorldRotation()) / 0.0174533f;
}

void EntityTransform::RigidbodySetPosition(b2Vec2 NewPosition)
{
	Position.x = NewPosition.x;
	Position.y = NewPosition.y;
	SetDirtyMatrixCache();
}

void EntityTransform::SetPosition(glm::vec3 NewPosition)
{

	Check(Parent);

	if (RigidbodyComponent* Rigidbody = Parent->GetComponent<RigidbodyComponent>())
	{
		if (glm::distance(Position, NewPosition) > .001f)
		{
			Rigidbody->SetPosition(b2Vec2(NewPosition.x - Rigidbody->GetOffset().x, NewPosition.y - Rigidbody->GetOffset().y));
		}
	}
	Position = NewPosition;

	SetDirtyMatrixCache();

}
glm::vec3 EntityTransform::GetWorldPosition()
{
	if (!AttachedEntity.Get())
	{
		return GetPosition();
	}
	else
	{
		return glm::vec3(AttachedEntity.Get()->GetTransform().GetModelMatrix() * glm::vec4(GetPosition(), 1.f));
	}
}

glm::vec3 EntityTransform::GetPosition()
{
	return Position;
}
void EntityTransform::SetScale(glm::vec3 NewScale)
{
	Scale = NewScale;

	//bModelMatrixDirty = true;
	//bXMirroredModelMatrixDirty = true;
	SetDirtyMatrixCache();
}
glm::vec3 EntityTransform::GetScale()
{
	return Scale;
}

glm::quat EntityTransform::GetWorldRotation()
{
	if (!AttachedEntity.Get())
	{
		return GetRotation();
	}
	else
	{
		return GetRotation() * glm::quat_cast(GetModelMatrix());
	}
}

void EntityTransform::SetRotation(glm::quat NewRotation)
{
	Rotation = NewRotation;
}

glm::quat EntityTransform::GetRotation()
{
	return Rotation;
}


glm::vec2 EntityTransform::ForwardVector2D()
{
	glm::vec4 TransformedResult = (glm::mat4_cast(GetWorldRotation()) * glm::vec4(1, 0, 0, 1));
	return glm::vec2(TransformedResult.x, TransformedResult.y);
}