#include "Character/Component/CharacterSanityComponent.h"
#include "Character/BaseCharacter.h"
#include "Engine/DamageEvents.h"
#include "Character/BasePlayerState.h"
#include "Kismet/GameplayStatics.h"

#include "LastCanary.h"

UCharacterSanityComponent::UCharacterSanityComponent()
{
	CurrentSpirit = MaxSpirit;
}

void UCharacterSanityComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCharacterSanityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}



float UCharacterSanityComponent::TakeSpiritDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	LOG_Char_WARNING(TEXT("캐릭터가 정신력에 타격을 받음"));
	if (!GetCharacter()->HasAuthority())
	{
		return 0;
	}
	ABasePlayerState* MyPlayerState = GetCharacter()->GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return 0;
	}
	if (MyPlayerState->bInfiniteSpirit == true)
	{
		return 0;
	}
	float FinalDamage = CalculateTakeSpiritDamage(DamageAmount);
	CurrentSpirit = FMath::Clamp(CurrentSpirit - FinalDamage, 0.0f, MaxSpirit);
	LOG_Char_WARNING(TEXT("Current Spirit : %f"), CurrentSpirit);
	if (CurrentSpirit <= PanicTriggerThreshold)
	{
		//: 정신력 낮음 처리
		EnterPanicState();
	}
	return DamageAmount;
}

void UCharacterSanityComponent::TriggerSpiritTickDamage()
{
	GetWorld()->GetTimerManager().SetTimer(
		SpiritTickDamageHandle,
		this,
		&UCharacterSanityComponent::TakeSpiritTickDamage,
		SpiritDamageTickInterval,
		true,           // 반복
		0.01f    // 처음 실행까지의 지연 시간
	);
}

void UCharacterSanityComponent::TakeSpiritTickDamage()
{
	FDamageEvent DamageEvent;
	float DamageAmount = SpiritTickDamage;
	AController* InstigatorController = GetPlayerController(); // 또는 nullptr
	AActor* DamageCauser = GetCharacter(); // 또는 원하는 액터

	TakeSpiritDamage(DamageAmount, DamageEvent, GetPlayerController(), DamageCauser);
}


float UCharacterSanityComponent::RestoreSpirit(float Amount)
{
	LOG_Char_WARNING(TEXT("캐릭터가 정신력을 회복함"));
	if (!GetCharacter()->HasAuthority())
	{
		return 0;
	}
	ABasePlayerState* MyPlayerState = GetCharacter()->GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return 0;
	}
	
	
	CurrentSpirit = FMath::Clamp(CurrentSpirit + Amount, 0.0f, MaxSpirit);
	
	LOG_Char_WARNING(TEXT("Current Spirit : %f"), CurrentSpirit);
	if (CurrentSpirit > MyPlayerState->PanicTriggerThreshold)
	{
		//: 정신력 높아짐 처리
		ExitPanicState();
	}
	return Amount;
}

float UCharacterSanityComponent::CalculateTakeSpiritDamage(float DamageAmount)
{
	//TODO: 여기에다가 추가로 뭔가 장비나 방어력이 추가 되면 여기서 계산하고 넘겨도 됨.
	return DamageAmount;
}

void UCharacterSanityComponent::EnterPanicState()
{
	//TODO: 서버에서의 처리
	LOG_Char_WARNING(TEXT("패닉 상태 진입"));

	//클라이언트에서의 처리
	Client_EnterPanicState();
}

void UCharacterSanityComponent::ExitPanicState()
{
	Client_ExitPanicState();
}


void UCharacterSanityComponent::Client_EnterPanicState_Implementation()
{
	StartPanicBehaviorLoop();
}

void UCharacterSanityComponent::Client_ExitPanicState_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(PanicActionTimerHandle);

}

void UCharacterSanityComponent::StartPanicBehaviorLoop()
{
	GetWorld()->GetTimerManager().SetTimer(
		PanicActionTimerHandle,
		this,
		&UCharacterSanityComponent::PerformRandomPanicAction,
		RepeatRate,
		true,           // 반복
		InitialDelay    // 처음 실행까지의 지연 시간
	);
}

void UCharacterSanityComponent::StopPanicBehaviorLoop()
{
	GetWorld()->GetTimerManager().ClearTimer(PanicActionTimerHandle);
}

void UCharacterSanityComponent::PerformRandomPanicAction()
{
	LOG_Char_WARNING(TEXT("패닉 행동 실행"));

	TArray<TFunction<void()>> PanicActions;

	PanicActions.Add([this]() { PlayScreamSound_Local(); });
	PanicActions.Add([this]() { TriggerPanicVoice(PanicDuration); });
	PanicActions.Add([this]() { UseItemUnexpectedly(); });
	PanicActions.Add([this]() { PlaySighSoundForAll(); });
	PanicActions.Add([this]() { ForceSetMouseSensitivity(PanicSensitivity, PanicDuration); });
	PanicActions.Add([this]() { ForceInvertMouseTemporary(true, PanicDuration); });

	// 랜덤 선택해서 실행
	if (PanicActions.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, PanicActions.Num() - 1);
		PanicActions[RandomIndex]();
	}

	//TODO: 정신력 0 처리

}

void UCharacterSanityComponent::PlayScreamSound_Local()
{
	if (GetCharacter()->IsLocallyControlled())
	{
		UGameplayStatics::PlaySoundAtLocation(this, ScreamSound, GetCharacter()->GetActorLocation());
	}
}

void UCharacterSanityComponent::UseItemUnexpectedly()
{
	LOG_Char_WARNING(TEXT("갑자기 아이템 사용"));

	GetCharacter()->UseEquippedItem(1.0f);
	GetCharacter()->UseEquippedItem(0.0f);
}

void UCharacterSanityComponent::PlaySighSoundForAll()
{
	if (GetCharacter()->IsLocallyControlled())
	{
		Server_PlaySighSound();
	}
}

void UCharacterSanityComponent::Server_PlaySighSound_Implementation()
{
	if (GetCharacter()->HasAuthority()) // 서버에서만 멀티캐스트 호출
	{
		Multicast_PlaySighSound();
	}
}


void UCharacterSanityComponent::Multicast_PlaySighSound_Implementation()
{
	if (SighSound)
	{
		UGameplayStatics::SpawnSoundAttached(
			SighSound,
			GetCharacter()->GetRootComponent(),         // 또는 GetMesh() 등 캐릭터에 붙일 컴포넌트
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			true                        // bStopWhenAttachedToDestroyed
		);

	}
}

void UCharacterSanityComponent::ForceSetMouseSensitivity(float NewSensitivity, float Duration)
{
	LOG_Char_WARNING(TEXT("마우스 반전"));

	GetCharacter()->MouseSensitivityMultiplier = 10.0f;
	// 기존 타이머 제거 후 새 타이머 시작
	GetWorld()->GetTimerManager().ClearTimer(MouseSensitivityRestoreHandle);
	GetWorld()->GetTimerManager().SetTimer(
		MouseSensitivityRestoreHandle,
		this,
		&UCharacterSanityComponent::RestoreOriginalMouseSensitivity,
		Duration,
		false
	);
}

void UCharacterSanityComponent::RestoreOriginalMouseSensitivity()
{
	GetCharacter()->MouseSensitivityMultiplier = 1.0f; // 초기화
}

void UCharacterSanityComponent::ForceInvertMouse(bool bInvert)
{
	GetCharacter()->MouseInvertMultiplier = bInvert ? -1.0f : 1.0f;
}

void UCharacterSanityComponent::ForceInvertMouseTemporary(bool bInvert, float Duration)
{
	// 반전 적용
	ForceInvertMouse(true);

	// 기존 타이머 제거 후 새로 시작
	GetWorld()->GetTimerManager().ClearTimer(MouseInvertResetTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		MouseInvertResetTimerHandle,
		this,
		&UCharacterSanityComponent::RestoreMouseInvert,
		Duration,
		false
	);
}

void UCharacterSanityComponent::RestoreMouseInvert()
{
	ForceInvertMouse(false);
}

void UCharacterSanityComponent::TriggerPanicVoice(float Duration)
{
	LOG_Char_WARNING(TEXT("보이스 변경"));

	EnterPanicVoice();
	GetWorld()->GetTimerManager().ClearTimer(PanicVoiceDurationHandle);
	GetWorld()->GetTimerManager().SetTimer(
		PanicVoiceDurationHandle,
		this,
		&UCharacterSanityComponent::ExitPanicVoice,
		Duration,
		false
	);
}