// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusChanged, AActor*, NewFocusedActor);


UCLASS()
class LASTCANARY_API UCharacterInteractionComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere)
    float TraceDistance = 500.f;

    UFUNCTION()
    void PerformTrace();

    UPROPERTY(BlueprintReadOnly)
    AActor* CurrentFocusedActor = nullptr;
    
    UFUNCTION()
    AActor* GetRecentInteractedActor() { return RecentInteractedActor; }

    UFUNCTION()
    void SetRecentInteractedActor(AActor* actor) { RecentInteractedActor = actor; }

    FOnFocusChanged OnFocusChanged;

    void Handle_Interact();
    
    void Interact();
    void Interact(AActor* Actor);
private:
    void UpdateFocus(AActor* NewActor);
    bool CanInteract();
    bool CheckInteractDirectly();

    AActor* RecentInteractedActor = nullptr;
};
