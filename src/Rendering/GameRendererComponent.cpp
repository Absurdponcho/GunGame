#include "GameRendererComponent.h"
#include "../Logging/Logging.h"

std::vector<GameRendererComponent*> GameRendererComponent::AllGameRendererComponents;

void GameRendererComponent::OnSpawn()
{
	GameComponent::OnSpawn();
	AllGameRendererComponents.push_back(this);
}

void GameRendererComponent::OnDestroy()
{
	GameComponent::OnDestroy();

	for (int Index = 0; Index < AllGameRendererComponents.size(); Index++)
	{
		if (*this == *AllGameRendererComponents[Index])
		{
			AllGameRendererComponents.erase(AllGameRendererComponents.begin() + Index);
			return;
		}
	}
}
void GameRendererComponent::RenderAllRenderers(std::stack<GameRendererComponent*>& VisibleRenderers)
{
	while (!VisibleRenderers.empty())
	{
		GameRendererComponent* RendererComponent = VisibleRenderers.top();
		if (RendererComponent->IsPendingDestroy() || !RendererComponent->IsEnabled())
		{
			continue;
		}
		RendererComponent->Render();
		VisibleRenderers.pop();
	}
}
void GameRendererComponent::CullAllRenderers(Out std::stack<GameRendererComponent*>& VisibleRenderers)
{
	for (int Index = 0; Index < AllGameRendererComponents.size(); Index++)
	{
		GameRendererComponent* RendererComponent = AllGameRendererComponents[Index];
		if (RendererComponent->IsPendingDestroy() || !RendererComponent->IsEnabled())
		{
			continue;
		}
		if (RendererComponent->Cull())
		{
			RendererComponent->bWasCulled = true;
		}
		else
		{
			RendererComponent->bWasCulled = false;
			VisibleRenderers.push(RendererComponent);
		}
	}
}