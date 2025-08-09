#include "Character/Component/CharacterNameWidgetComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/UIObject/PlayerNameWidget.h"
#include "Character/BaseCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Character/BasePlayerState.h"
#include "LastCanary.h"

UCharacterNameWidgetComponent::UCharacterNameWidgetComponent()
{
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidget"));
}

void UCharacterNameWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterNameWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!WidgetComponent) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerCameraManager* CamManager = UGameplayStatics::GetPlayerCameraManager(World, 0);
	if (!CamManager) return;

	FVector CameraLocation = CamManager->GetCameraLocation();
	FVector WidgetLocation = WidgetComponent->GetComponentLocation();

	float Distance = FVector::Dist(CameraLocation, WidgetLocation);
	if (Distance < MaxVisibleDistance)
	{
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(WidgetLocation, CameraLocation);
		FRotator YawOnly = FRotator(0.f, LookAtRotation.Yaw, 0.f);
		WidgetComponent->SetWorldRotation(YawOnly);
	}
}

void UCharacterNameWidgetComponent::InitializeWidget()
{
	if (!GetCharacter()) return;
	
	USkeletalMeshComponent* Mesh = GetCharacter()->GetMesh();
	if (!Mesh) return;

	if (!WidgetComponent) return;

	if (!WidgetComponent->GetWidgetClass())
	{
		WidgetComponent->SetWidgetClass(DefaultWidgetClass);
	}

	// 필수 설정
	WidgetComponent->RegisterComponent();
	WidgetComponent->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);
	WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
	WidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	WidgetComponent->SetDrawSize(FVector2D(200, 50));
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WidgetComponent->SetIsReplicated(false);
	WidgetComponent->SetTickWhenOffscreen(true);
	WidgetComponent->SetTwoSided(true);
	WidgetComponent->SetUsingAbsoluteRotation(false);
	WidgetComponent->SetPivot(FVector2D(0.5f, 0.5f));

	// 커스텀 뎁스 설정
	WidgetComponent->SetRenderCustomDepth(true);
	WidgetComponent->SetCustomDepthStencilValue(1);
}

void UCharacterNameWidgetComponent::UpdateRotation()
{
}

void UCharacterNameWidgetComponent::ApplyName()
{
}

void UCharacterNameWidgetComponent::UpdateWidget()
{
}

void UCharacterNameWidgetComponent::TurnOffWidget()
{
}

UUserWidget* UCharacterNameWidgetComponent::GetWidget() const
{
	return WidgetComponent ? WidgetComponent->GetWidget() : nullptr;
}

void UCharacterNameWidgetComponent::SetPlayerName(const FString& PlayerName)
{
	if (!WidgetComponent) return;
	// 로컬에서 설정
	if (UPlayerNameWidget* NameWidget = Cast<UPlayerNameWidget>(WidgetComponent->GetWidget()))
	{
		NameWidget->SetPlayerName(PlayerName);
	}

	// 클라이언트라면 서버에 요청
	if (GetOwner()->GetLocalRole() < ROLE_Authority)
	{
		Server_SetPlayerName(PlayerName);
	}
}

void UCharacterNameWidgetComponent::SetWidgetVisibility(bool bVisible)
{
	if (WidgetComponent)
	{
		WidgetComponent->SetVisibility(bVisible, true);
	}
}

void UCharacterNameWidgetComponent::SetCastShadowEnabled(bool bEnable)
{
	if (WidgetComponent)
	{
		WidgetComponent->SetCastShadow(bEnable);
		WidgetComponent->CastShadow = bEnable;
	}
}

void UCharacterNameWidgetComponent::InitializeNameWidget()
{
	if (!GetCharacter()) return;

	APlayerController* PC = Cast<APlayerController>(GetCharacter()->GetInstigatorController());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	APlayerState* PS = GetCharacter()->GetPlayerState();
	if (PS && WidgetComponent)
	{
		if (IsValid(GetWidget()))
		{
			SetPlayerName(PS->GetPlayerName());
			

			if (GetCharacter()->IsLocallyControlled())
			{
				SetWidgetVisibility(false);
			}

			SetCastShadowEnabled(false);
		}

		// UpdateNameWidget 역할: 위젯 업데이트 등 필요한 초기화 로직 호출
		// 예) SetPlayerName(PS->GetPlayerName());

		// 위젯 숨기기 등
		LOG_Char_WARNING(TEXT("셋 비저빌리티"));

		WidgetComponent->SetVisibility(false, true);
	}
	else
	{
		LOG_Char_WARNING(TEXT("타이머 재시도"));

		// 준비 안 된 경우 타이머 재시도
		if (UWorld* World = GetCharacter()->GetWorld())
		{
			World->GetTimerManager().SetTimer(
				RetryInitializeHandle,
				this,
				&UCharacterNameWidgetComponent::InitializeNameWidget,
				0.2f,
				false);
		}
	}
}

void UCharacterNameWidgetComponent::UpdateNameFromPlayerState(APlayerState* PlayerState)
{
	if (!PlayerState || !IsValid(WidgetComponent)) return;

	if (UPlayerNameWidget* NameWidget = Cast<UPlayerNameWidget>(WidgetComponent->GetWidget()))
	{
		NameWidget->SetPlayerName(PlayerState->GetPlayerName());
	}

	// 서버라면 플레이어의 인게임 이름도 갱신
	if (GetOwner()->HasAuthority())
	{
		if (ABasePlayerState* BPS = Cast<ABasePlayerState>(PlayerState))
		{
			BPS->SetPlayerInGameName(PlayerState->GetPlayerName());
		}
	}
}

void UCharacterNameWidgetComponent::HideNameWidget()
{
	if (IsValid(WidgetComponent))
	{
		WidgetComponent->SetVisibility(false, true);
	}
}

void UCharacterNameWidgetComponent::TryInitializeOnPlayerState(APlayerState* PS, bool bIsLocallyControlled, bool bHasAuthority)
{
	UpdateNameFromPlayerState(PS);

	// 로컬이면 위젯 비활성화
	if (bIsLocallyControlled && IsValid(WidgetComponent))
	{
		WidgetComponent->SetVisibility(false, true);
		WidgetComponent->SetCastShadow(false);
		WidgetComponent->CastShadow = false;
	}
}

void UCharacterNameWidgetComponent::Multicast_SetPlayerName_Implementation(const FString& PlayerName)
{
	if (!WidgetComponent) return;

	if (UPlayerNameWidget* NameWidget = Cast<UPlayerNameWidget>(WidgetComponent->GetWidget()))
	{
		NameWidget->SetPlayerName(PlayerName);
	}
}

bool UCharacterNameWidgetComponent::Server_SetPlayerName_Validate(const FString& PlayerName)
{
	return true;
}

void UCharacterNameWidgetComponent::Server_SetPlayerName_Implementation(const FString& PlayerName)
{
	// 서버에서 멀티캐스트 호출
	Multicast_SetPlayerName(PlayerName);
}