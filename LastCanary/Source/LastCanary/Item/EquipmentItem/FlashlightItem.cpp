// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/EquipmentItem/FlashlightItem.h"
#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "AI/EliteMonster/CaveEliteMonster.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "LastCanary.h"

AFlashlightItem::AFlashlightItem()
{
    // 메인 스포트라이트
    SpotLightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
    SpotLightComponent->SetupAttachment(StaticMeshComponent, TEXT("SpotLight"));
    SpotLightComponent->SetIsReplicated(true);
    SpotLightComponent->SetVisibility(false);

    // 손전등의 On, Off 여부를 확인하기 위한 스포트라이트
    GlassGlowComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("GlassGlow"));
    GlassGlowComponent->SetupAttachment(StaticMeshComponent, TEXT("GlassGlow"));
    GlassGlowComponent->SetIsReplicated(true);
    GlassGlowComponent->SetVisibility(false);

    bIsLightOn = false;

    //몬스터 이벤트
    LightDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("LightDetectionSphere"));
    LightDetectionSphere->SetupAttachment(RootComponent);
    LightDetectionSphere->SetSphereRadius(1000.0f);
    LightDetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    LightDetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    LightDetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    LightDetectionSphere->SetIsReplicated(true);

    //오버랩
    LightDetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlashlightItem::OnLightOverlapBegin);
    LightDetectionSphere->OnComponentEndOverlap.AddDynamic(this, &AFlashlightItem::OnLightOverlapEnd);
}


void AFlashlightItem::OnLightOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsLightOn) return;
    if (!HasAuthority()) return; // 서버에서만 처리

    if (ACaveEliteMonster* EliteMonster = Cast<ACaveEliteMonster>(OtherActor))
    {
        /*EliteMonster->FreezeAI();*/
        UE_LOG(LogTemp, Error, TEXT("아무튼 작동됨!"));
    }
}

void AFlashlightItem::OnLightOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!bIsLightOn) return;
    if (!HasAuthority()) return;

    if (ACaveEliteMonster* EliteMonster = Cast<ACaveEliteMonster>(OtherActor))
    {
        /*EliteMonster->UnfreezeAI();*/
        UE_LOG(LogTemp, Error, TEXT("아무튼 작동끝!"));
    }
}

void AFlashlightItem::UseItem()
{
    // Null 체크 추가
    if (!SpotLightComponent)
    {
        LOG_Item_ERROR(TEXT("[AFlashlightItem::UseItem] SpotLightComponent가 유효하지 않습니다!"));
        return;
    }

    // 내구도 유효성 검사 강화
    if (FMath::IsNearlyZero(Durability) || Durability < 0.0f)
    {
        Durability = 0.0f;
        bIsLightOn = false;
        SpotLightComponent->SetVisibility(false);
        LOG_Item_WARNING(TEXT("[AFlashlightItem::UseItem] 배터리가 완전히 소진되었습니다."));
        return;
    }

    PlaySoundByType();
    // 빛 상태 토글
    bIsLightOn = bIsSoundActive;

    // 네트워크 동기화 처리
    if (GetLocalRole() == ROLE_Authority)
    {
        Multicast_UpdateLightState(bIsLightOn);
    }
    else
    {
        Server_ToggleLight(bIsLightOn);
    }

    // 부모 클래스 기능 호출
    //Super::UseItem();
    OnItemStateChanged.Broadcast();
}

void AFlashlightItem::Server_ToggleLight_Implementation(bool bNewState)
{
    // 서버에서 배터리 상태 재확인
    if (bNewState && Durability <= 0.0f)
    {
        // 배터리 부족 시, 켤 수 없음
        return;
    }

    // 상태 변경 및 모든 클라이언트에 전파
    bIsLightOn = bNewState;
    Multicast_UpdateLightState(bIsLightOn);
    return;
}

void AFlashlightItem::Multicast_UpdateLightState_Implementation(bool bNewState)
{
    // 상태 업데이트
    bIsLightOn = bNewState;

    // 빛 컴포넌트 유효성 검사 로그
    if (!SpotLightComponent)
    {
        LOG_Item_WARNING(TEXT("[AFlashlightItem::Multicast_UpdateLightState] SpotLightComponent가 유효하지 않음!"));
        return;
    }

    // 빛 가시성 업데이트
    SpotLightComponent->SetVisibility(bIsLightOn);
    GlassGlowComponent->SetVisibility(bIsLightOn);

    // 빛 강도 설정
    if (!bIsLightOn)
    {
        SpotLightComponent->SetIntensity(0.0f);
    }
    else
    {
        // 모든 속성 명시적 설정 (클라이언트에서도)
        SpotLightComponent->SetIntensity(LightIntensity);
        SpotLightComponent->SetLightColor(LightColor);
        SpotLightComponent->SetInnerConeAngle(InnerConeAngle);
        SpotLightComponent->SetOuterConeAngle(OuterConeAngle);

        // 중요: 빛 효과 강제 업데이트
        SpotLightComponent->UpdateColorAndBrightness();
    }

    // 타이머 관리
    if (bIsLightOn && Durability > 0.0f)
    {
        if (HasAuthority())  // 서버에서만 타이머 설정
        {
            if (GetWorld())
            {
                GetWorld()->GetTimerManager().SetTimer(BatteryTimerHandle, this, &AFlashlightItem::ConsumeBattery, 0.1f, true);
                // 즉시 한 번 호출하여 로그 확인
                ConsumeBattery();
            }
            else
            {
                LOG_Item_WARNING(TEXT("[AFlashlightItem::Multicast_UpdateLightState] World가 유효하지 않음!"));
            }
        }
    }
    else
    {
        if (HasAuthority())  // 서버에서만 타이머 해제
        {
            if (GetWorld())
            {
                GetWorld()->GetTimerManager().ClearTimer(BatteryTimerHandle);
            }
            else
            {
                LOG_Item_WARNING(TEXT("[AFlashlightItem::Multicast_UpdateLightState] 타이머 해제 시 World가 유효하지 않음!"));
            }
        }
    }
}

void AFlashlightItem::SetEquipped(bool bNewEquipped)
{
    Super::SetEquipped(bNewEquipped);

    // 장착 해제시 자동으로 불 꺼짐
    if (!bNewEquipped && bIsLightOn)
    {
        bIsLightOn = false;
        SpotLightComponent->SetVisibility(bIsLightOn);
        GlassGlowComponent->SetVisibility(bIsLightOn);


        if (GetWorld() && BatteryTimerHandle.IsValid())
        {
            GetWorld()->GetTimerManager().ClearTimer(BatteryTimerHandle);
        }
    }
}

void AFlashlightItem::ToggleLight(bool bTurnOn)
{
    if (bTurnOn && Durability <= 0.0f)
    {
        // 배터리가 없으면 켤 수 없음
        return;
    }

    bIsLightOn = bTurnOn;
    SpotLightComponent->SetVisibility(bIsLightOn);

    // 상태 변경 알림
    OnItemStateChanged.Broadcast();
}

void AFlashlightItem::ConsumeBattery()
{
    if (!bIsLightOn || !bIsEquipped)
    {
        return;
    }

    bool bShouldConsume = false;

    if (GetLocalRole() == ROLE_Authority)
    {
        // 서버에서는 소유자가 로컬인지 확인
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            bShouldConsume = OwnerPawn->IsLocallyControlled();
        }
    }
    else if (GetLocalRole() == ROLE_AutonomousProxy)
    {
        // 클라이언트에서는 자신의 아이템인지 확인
        bShouldConsume = true;
    }

    if (bShouldConsume)
    {
        Durability = FMath::Clamp(Durability - (BatteryConsumptionRate * 0.1f), 0.0f, 100.0f);

        // 배터리가 완전히 소모될 때만 서버에 알림
        if (Durability <= 0.0f)
        {
            if (GetLocalRole() != ROLE_Authority)
            {
                Server_SetBatteryDepleted();
            }
            else
            {
                // 서버에서는 직접 처리
                Multicast_UpdateLightState(false);
                OnItemStateChanged.Broadcast();
            }
        }
    }
}

void AFlashlightItem::Server_SetBatteryDepleted_Implementation()
{
    Durability = 0.0f;
    Multicast_UpdateLightState(false);
    OnItemStateChanged.Broadcast();
}

void AFlashlightItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AFlashlightItem, bIsLightOn);
}

void AFlashlightItem::BeginPlay()
{
    Super::BeginPlay();

    // 스포트라이트 속성 적용
    SpotLightComponent->SetIntensity(LightIntensity);
    SpotLightComponent->SetLightColor(LightColor);
    SpotLightComponent->SetInnerConeAngle(InnerConeAngle);
    SpotLightComponent->SetOuterConeAngle(OuterConeAngle);

    if (SpotLightComponent && !SpotLightComponent->GetIsReplicated())
    {
        SpotLightComponent->SetIsReplicated(true);
    }

    if (LightDetectionSphere)
    {
        LightDetectionSphere->SetHiddenInGame(false);
        LightDetectionSphere->SetVisibility(true);
    }
}

void AFlashlightItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetWorld() && BatteryTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(BatteryTimerHandle);
    }

    Super::EndPlay(EndPlayReason);
}
