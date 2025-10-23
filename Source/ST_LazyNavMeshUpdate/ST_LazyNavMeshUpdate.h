#pragma once

#include "Modules/ModuleManager.h"

// the purpose of this plugin is to disable navigation rebuilds while moving actors in the viewport
// the navigation rebuild will be locked until the move action was finished (mouse up)
// then the actor bounds on begin and end move will be marked dirty on the navmesh, causing it to rebuild

class FST_LazyNavMeshUpdateModule : public IModuleInterface
{
public:

	static bool bLazyUpdates;
	
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void OnBeginMove(UObject& Object);
	void OnEndMove(UObject& Object);
	
	void AddToolbarExtension(FToolBarBuilder& ToolbarBuilder);

	
private:
	
	// tracks actor bounds from the begin move event
	TMap<TWeakObjectPtr<AActor>, FBox> ActorBoundsCache;
	
	static FBox GetActorBounds(UObject *Object);
};
