
#include "Multi/VoiceChat/ProximityVoiceChatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "Net/VoiceConfig.h"


UProximityVoiceChatComponent::UProximityVoiceChatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UProximityVoiceChatComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeVoiceChat();
}

void UProximityVoiceChatComponent::ToggleSpeaking(bool bEnable)
{
	if (!GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("VoiceChatComponent: Owner is null!"));
		return;
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetOwner(), 0))
	{
		FString Command = FString::Printf(TEXT("ToggleSpeaking %d"), bEnable ? 1 : 0);
		PC->ConsoleCommand(Command);
	}
}

void UProximityVoiceChatComponent::InitializeVoiceChat()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn)
	{
		APlayerState* PS = Pawn->GetPlayerState();
		if (PS)
		{
			VOIPTalker = UVOIPTalker::CreateTalkerForPlayer(PS);
			if (VOIPTalker)
			{
				VOIPTalker->RegisterWithPlayerState(PS);
			}
		}
	}
	if (!VOIPTalker)
	{
		UE_LOG(LogTemp, Warning, TEXT("VOIPTalker creation failed."));
		return;
	}

	// Delay 대신 타이머로 PlayerState 등록 지연
	FTimerHandle TimerHandle;
	Owner->GetWorldTimerManager().SetTimer(TimerHandle, [this, Owner]()
	{
		APawn* OwnerPawn = Cast<APawn>(Owner);
		if (!OwnerPawn) return;

		APlayerState* PS = OwnerPawn->GetPlayerState();
		if (VOIPTalker && PS)
		{
			VOIPTalker->RegisterWithPlayerState(PS);
			ConfigureVoiceSettings();
		}

		// 로컬 컨트롤러일 경우 루프백
		if (OwnerPawn->IsLocallyControlled())
		{
			UGameplayStatics::GetPlayerController(Owner, 0)->ConsoleCommand(TEXT("OSS.VoiceLoopback 1"));
		}

	}, 0.2f, false);
	
}

void UProximityVoiceChatComponent::ConfigureVoiceSettings()
{
	FVoiceSettings VoiceSettings;
	VoiceSettings.ComponentToAttachTo = GetOwner()->FindComponentByClass<UCapsuleComponent>();
	VoiceSettings.AttenuationSettings = AttenuationSettings;
	VoiceSettings.SourceEffectChain = SourceEffectChain;
	
	VOIPTalker->Settings= VoiceSettings;
}