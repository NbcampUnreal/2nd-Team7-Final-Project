#include "Character/AnimNotifyState/LCSoundNotifyState.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Character/BaseCharacter.h"

void ULCSoundNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner) || !SoundToPlay)
	{
		return;
	}

	FVector Location = Owner->GetActorLocation();

	if (bLoopSound) // 루프 사운드를 위한 AudioComponent 생성
	{
		PlayingAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
			Owner,
			SoundToPlay,
			Location,
			FRotator::ZeroRotator,
			1.0f,
			1.0f,
			0.0f,
			nullptr,
			nullptr,
			true // bAutoDestroy: false로 지정해야 Stop 후에도 컴포넌트가 살아있음
		);
	}
	else
	{
		UGameplayStatics::PlaySoundAtLocation(Owner, SoundToPlay, Location);
	}
}

void ULCSoundNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (bLoopSound && IsValid(PlayingAudioComponent))
	{
		PlayingAudioComponent->Stop();
		PlayingAudioComponent = nullptr;
	}
}