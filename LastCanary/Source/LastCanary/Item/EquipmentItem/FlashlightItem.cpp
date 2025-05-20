// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/EquipmentItem/FlashlightItem.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

AFlashlightItem::AFlashlightItem()
{
    // 스포트라이트 컴포넌트 생성
    SpotLightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
    SpotLightComponent->SetupAttachment(MeshComponent);

    // 스포트라이트 위치 조정 (메시의 앞쪽을 향하도록)
    SpotLightComponent->SetRelativeLocation(FVector(50.0f, 0.0f, 0.0f));
    SpotLightComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

    // 기본 속성 설정
    LightIntensity = 5000.0f;
    LightColor = FLinearColor(1.0f, 0.9f, 0.8f);  // 따뜻한 빛
    InnerConeAngle = 20.0f;
    OuterConeAngle = 35.0f;
    BatteryConsumptionRate = 0.1f;  // 초당 배터리 소모율 (Durability 기준)

    // 초기에는 빛 꺼두기
    bIsLightOn = false;
    SpotLightComponent->SetVisibility(false);

    // 기본 내구도를 배터리 용량으로 활용
    Durability = 100.0f;  // 100%
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

    // 빛 상태 토글
    bIsLightOn = !bIsLightOn;

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
    Multicast_UpdateLightState(bNewState);
}

void AFlashlightItem::Multicast_UpdateLightState_Implementation(bool bNewState)
{
    bIsLightOn = bNewState;
    SpotLightComponent->SetVisibility(bIsLightOn);

    // 내구도 소모 시작/중지
    if (bIsLightOn)
    {
        GetWorld()->GetTimerManager().SetTimer(BatteryTimerHandle, this, &AFlashlightItem::ConsumeBattery, 0.1f, true);
    }
    else
    {
        GetWorld()->GetTimerManager().ClearTimer(BatteryTimerHandle);
    }
}

void AFlashlightItem::SetEquipped(bool bNewEquipped)
{
    Super::SetEquipped(bNewEquipped);

    // 장착 해제시 자동으로 불 꺼짐
    if (!bNewEquipped && bIsLightOn)
    {
        bIsLightOn = false;
        SpotLightComponent->SetVisibility(false);
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

void AFlashlightItem::ActivateBatteryConsumption()
{
    if (bIsLightOn)
    {
        GetWorld()->GetTimerManager().SetTimer(BatteryTimerHandle, this, &AFlashlightItem::ConsumeBattery, 0.5f, true);
    }
    else
    {
        GetWorld()->GetTimerManager().ClearTimer(BatteryTimerHandle);
    }
}

void AFlashlightItem::ConsumeBattery()
{
    if (!bIsLightOn || !bIsEquipped) return;

    // 서버 권한 확인
    if (GetLocalRole() != ROLE_Authority) return;

    Durability = FMath::Clamp(Durability - (BatteryConsumptionRate * 0.1f), 0.0f, 100.0f);

    if (Durability <= 0.0f)
    {
        Multicast_UpdateLightState(false);
        OnItemStateChanged.Broadcast();
    }
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
}
