#include "ST_LazyNavMeshUpdate.h"

#include "LevelEditor.h"
#include "LevelEditorSubsystem.h"
#include "NavigationSystem.h"

#define LOCTEXT_NAMESPACE "FST_LazyNavMeshUpdateModule"

bool FST_LazyNavMeshUpdateModule::bLazyUpdates = true;

void FST_LazyNavMeshUpdateModule::StartupModule()
{
	GEditor->OnBeginObjectMovement().AddRaw(this, &FST_LazyNavMeshUpdateModule::OnBeginMove);
	GEditor->OnEndObjectMovement().AddRaw(this, &FST_LazyNavMeshUpdateModule::OnEndMove);

	bLazyUpdates = GConfig->GetBool(TEXT("/Script/ST_LazyNavMeshUpdate.ST_LazyNavMeshUpdateSettings"), TEXT("bLazyUpdates"), bLazyUpdates, GEditorPerProjectIni);
	

	// viewport overlay button
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Transform", EExtensionHook::After, nullptr, FToolBarExtensionDelegate::CreateRaw(this, &FST_LazyNavMeshUpdateModule::AddToolbarExtension));
	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
}



void FST_LazyNavMeshUpdateModule::AddToolbarExtension(FToolBarBuilder& ToolbarBuilder)
{	
	ToolbarBuilder.BeginSection("ST_LazyNavMeshUpdate");	
	ToolbarBuilder.AddToolBarButton(
	FUIAction(
		FExecuteAction::CreateLambda([]()
		{
			bLazyUpdates = !bLazyUpdates;
			GConfig->SetBool(TEXT("/Script/ST_LazyNavMeshUpdate.ST_LazyNavMeshUpdateSettings"), TEXT("bLazyUpdates"), bLazyUpdates, GEditorPerProjectIni);
			GConfig->Flush(false, GEditorPerProjectIni); // write to disk
		}),
			FCanExecuteAction::CreateLambda([]() { return true; }),
			FIsActionChecked::CreateLambda([](){ return !bLazyUpdates; } )),
			NAME_None,
		FText::GetEmpty(),
		MakeAttributeLambda([]() -> FText { return bLazyUpdates ? FText::FromString("lazy nav mesh updates enabled") : FText::FromString("lazy nav mesh updates disabled"); }), 
	   FSlateIcon(FAppStyle::GetAppStyleSetName(), "ShowFlagsMenu.Navigation"),  // icon
	   EUserInterfaceActionType::ToggleButton
	);
	ToolbarBuilder.EndSection();
}

void FST_LazyNavMeshUpdateModule::ShutdownModule()
{
	// yolo
}

void FST_LazyNavMeshUpdateModule::OnBeginMove(UObject& Object)
{
	if(!bLazyUpdates)
		return;
	
	if (UWorld* World = Object.GetWorld())
	{
		if(World->WorldType != EWorldType::Editor)
			return;
		
		if (auto* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
		{
			NavSys->AddNavigationBuildLock(ENavigationBuildLock::Custom);

			const FBox ActorBounds = GetActorBounds(&Object);
			if(ActorBounds.IsValid && ActorBounds.GetSize().Length() > 0)
			{
				//UE_LOG(LogTemp, Warning, TEXT("[FST_LazyNavMeshUpdateModule::OnBeginMove] LOCK NAV REBUILD for %s"), *Object.GetName());
				BoundsBeginMove = ActorBounds;				
			}		
		}
	}
}

void FST_LazyNavMeshUpdateModule::OnEndMove(UObject& Object)
{
	if(!bLazyUpdates)
		return;
		
	if (UWorld* World = Object.GetWorld())
	{
		if(World->WorldType != EWorldType::Editor)
			return;
				
		if (auto* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
		{
			//UE_LOG(LogTemp, Warning, TEXT("[FST_LazyNavMeshUpdateModule::OnEndMove] UNLOCK NAV REBUILD for %s"), *Object.GetName());
			NavSys->RemoveNavigationBuildLock(ENavigationBuildLock::Custom, UNavigationSystemV1::ELockRemovalRebuildAction::NoRebuild);

			const FBox ActorBounds = GetActorBounds(&Object);
			if(ActorBounds.IsValid && ActorBounds.GetSize().Length() > 0)
			{
				NavSys->AddDirtyArea(ActorBounds, ENavigationDirtyFlag::NavigationBounds);
			}
			
			if(FBox *BoundsBegin = BoundsBeginMove.GetPtrOrNull())
			{
				NavSys->AddDirtyArea(*BoundsBegin, ENavigationDirtyFlag::NavigationBounds);	
				BoundsBeginMove.Reset();
			}									
		}
	}
}


FBox FST_LazyNavMeshUpdateModule::GetActorBounds(UObject* Object)
{
	if(AActor *AsActor = Cast<AActor>(Object))
	{
		FVector Origin = FVector::ZeroVector;
		FVector Extent = FVector::ZeroVector;
		AsActor->GetActorBounds(true, Origin, Extent, true);

		FBox Bounds = FBox(Origin-Extent, Origin+Extent);
		return Bounds;
	}

	if(USceneComponent *AsSceneComponent = Cast<USceneComponent>(Object)){
		return GetActorBounds(AsSceneComponent->GetOwner());
	}

	return FBox(NoInit);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FST_LazyNavMeshUpdateModule, ST_LazyNavMeshUpdate)