// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/EquipmentItem/FlashlightItem.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "LastCanary.h"

AFlashlightItem::AFlashlightItem()
{
    // 스포트라이트 컴포넌트 생성
    SpotLightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
    SpotLightComponent->SetupAttachment(MeshComponent);

    bReplicates = true;
    // bAlwaysRelevant = true; // 이 옵션을 제거하거나 아래 옵션으로 대체

    // 대안: 소유자에게는 항상 관련 있게, 다른 플레이어에게는 관련성 기반으로 설정
    bOnlyRelevantToOwner = false;
    NetUpdateFrequency = 0.01f; // 업데이트 빈도 제한 (초당 5회)


    // 네트워크 복제 활성화
    SpotLightComponent->SetIsReplicated(true);

    // 스포트라이트 위치 조정 (메시의 앞쪽을 향하도록)
    SpotLightComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 35.0f));
    SpotLightComponent->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));

    // 기본 속성 설정
    LightIntensity = 5000.0f;
    LightColor = FLinearColor(1.0f, 0.9f, 0.8f);  // 따뜻한 빛
    InnerConeAngle = 20.0f;
    OuterConeAngle = 35.0f;
    BatteryConsumptionRate = 0.2f;  // 초당 배터리 소모율 (Durability 기준)

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
    UE_LOG(LogTemp, Warning, TEXT("[AFlashlightItem::Multicast_UpdateLightState] 실행 - 상태: %s, Role: %d, RemoteRole: %d"),
        bNewState ? TEXT("켜짐") : TEXT("꺼짐"),
        (int32)GetLocalRole(),
        (int32)GetRemoteRole());
    // 상태 업데이트
    bIsLightOn = bNewState;

    // 빛 컴포넌트 유효성 검사 로그
    if (!SpotLightComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("[AFlashlightItem::Multicast_UpdateLightState] SpotLightComponent가 유효하지 않음!"));
        return;
    }

    // 빛 가시성 업데이트
    SpotLightComponent->SetVisibility(bIsLightOn);

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

    // 사운드 재생
    Multicast_PlayFlashlightSound(bIsLightOn);

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
                UE_LOG(LogTemp, Error, TEXT("[AFlashlightItem::Multicast_UpdateLightState] World가 유효하지 않음!"));
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
                UE_LOG(LogTemp, Error, TEXT("[AFlashlightItem::Multicast_UpdateLightState] 타이머 해제 시 World가 유효하지 않음!"));
            }
        }
    }

    if (GetLocalRole() != ROLE_Authority)
    {
        UE_LOG(LogTemp, Warning, TEXT("[클라이언트] 손전등 상태 변경: %s, 컴포넌트 가시성: %d"),
            bNewState ? TEXT("켜짐") : TEXT("꺼짐"),
            SpotLightComponent->IsVisible());
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

void AFlashlightItem::ConsumeBattery()
{
    if (!bIsLightOn || !bIsEquipped)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AFlashlightItem::ConsumeBattery] 손전등 상태 확인: bIsLightOn=%d, bIsEquipped=%d"),
            bIsLightOn, bIsEquipped);
        return;
    }

    // 서버 권한 확인
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    Durability = FMath::Clamp(Durability - (BatteryConsumptionRate * 0.1f), 0.0f, 100.0f);

    // 화면에도 배터리 상태 표시 (디버깅용)
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow,
            FString::Printf(TEXT("배터리: %.1f%%"), Durability));
    }

    if (Durability <= 0.0f)
    {
        Multicast_UpdateLightState(false);
        OnItemStateChanged.Broadcast();
    }
}

void AFlashlightItem::Multicast_PlayFlashlightSound_Implementation(bool bTurnOn)
{
    // 재생할 사운드 선택
    USoundBase* SoundToPlay = bTurnOn ? TurnOnSound : TurnOffSound;

    // 사운드가 설정되었는지 확인
    if (!SoundToPlay)
    {
        UE_LOG(LogTemp, Warning, TEXT("손전등 사운드가 설정되지 않았습니다!"));
        return;
    }

    // 감쇠 설정 적용
    UGameplayStatics::PlaySoundAtLocation(
        this,
        SoundToPlay,
        GetActorLocation(),
        SoundVolume,
        1.0f,
        0.0f,
        FlashlightSoundAttenuation  // 에디터에서 감쇠 설정 전달
    );
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

    // 사운드 에셋 동적 로드
    if (!TurnOnSound)
    {
        TurnOnSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/AnotherAsset/Item/Flashlight/Sounds/Flash_Switch-on_cut.Flash_Switch-on_cut"));
        if (!TurnOnSound)
        {
            UE_LOG(LogTemp, Warning, TEXT("[AFlashlightItem] 손전등 켜기 사운드 로드 실패"));
        }
    }

    if (!TurnOffSound)
    {
        TurnOffSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/AnotherAsset/Item/Flashlight/Sounds/Flash_Switch_Off.Flash_Switch_Off"));
        if (!TurnOffSound)
        {
            UE_LOG(LogTemp, Warning, TEXT("[AFlashlightItem] 손전등 끄기 사운드 로드 실패"));
        }
    }

    if (!FlashlightSoundAttenuation)
    {
        FlashlightSoundAttenuation = LoadObject<USoundAttenuation>(nullptr, TEXT("/Game/AnotherAsset/Item/Flashlight/Sounds/ATT_FlashlightSound.ATT_FlashlightSound"));
    }

    if (SpotLightComponent && !SpotLightComponent->GetIsReplicated())
    {
        UE_LOG(LogTemp, Warning, TEXT("[AFlashlightItem::BeginPlay] SpotLightComponent 복제 활성화"));
        SpotLightComponent->SetIsReplicated(true);
    }
}
