#include "Item/Component/DamageReceiverComponent.h"
#include "Actor/TrainingRoom/StaticTrainingDummy.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayTagAssetInterface.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "LastCanary.h"

UDamageReceiverComponent::UDamageReceiverComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;

    // 네트워크 복제 활성화
    SetIsReplicated(true);

    CurrentHealth = MaxHealth;

    // 기본 머티리얼 태그 설정
    MaterialTag = FGameplayTag::RequestGameplayTag(TEXT("Material.Metal"));
}

void UDamageReceiverComponent::BeginPlay()
{
    Super::BeginPlay();

    // 서버에서만 초기화 수행
    if (GetOwner()->HasAuthority())
    {
        // 초기 체력 설정
        CurrentHealth = MaxHealth;

        // 자동으로 Enemy 태그 추가
        if (bAutoAddEnemyTag)
        {
            AddGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Character.Enemy")));
        }

        // 머티리얼 태그 추가
        if (MaterialTag.IsValid())
        {
            AddGameplayTag(MaterialTag);
        }
    }

    // 기본 헤드샷 본 이름 설정 (설정되지 않았을 경우)
    if (HeadshotBoneNames.Num() == 0)
    {
        HeadshotBoneNames.Add(FName("Head"));
        HeadshotBoneNames.Add(FName("head"));
        HeadshotBoneNames.Add(FName("Head_joint"));
        HeadshotBoneNames.Add(FName("head_joint"));
    }

    // 오너 액터의 데미지 시스템과 연결
    SetupOwnerDamageBinding();

    // 충돌 설정
    if (bAutoSetupCollision)
    {
        SetupCollisionSettings();
    }
}

void UDamageReceiverComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION_NOTIFY(UDamageReceiverComponent, CurrentHealth, COND_None, REPNOTIFY_Always);
}

void UDamageReceiverComponent::OnRep_CurrentHealth()
{
    // 클라이언트에서 체력 변화 시 이벤트 처리
    LOG_Item_WARNING(TEXT("[DamageReceiver] 체력 변경: %.1f/%.1f"), CurrentHealth, MaxHealth);

    // 체력이 0 이하가 되면 처리
    if (CurrentHealth <= 0.0f && GetOwner()->HasAuthority())
    {
        HandleHealthDepleted();
    }
}

void UDamageReceiverComponent::SetupOwnerDamageBinding()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // 오너가 IGameplayTagAssetInterface를 구현하지 않는 경우를 위한 처리
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Owner);
    if (!TagInterface)
    {
        LOG_Item_WARNING(TEXT("[DamageReceiver] 오너 액터가 IGameplayTagAssetInterface를 구현하지 않음. 수동으로 TakeDamage를 오버라이드해야 함."));
    }
}

void UDamageReceiverComponent::SetupCollisionSettings()
{
    UMeshComponent* MeshComp = FindTargetMeshComponent();
    if (!MeshComp)
    {
        LOG_Item_WARNING(TEXT("[DamageReceiver] 대상 메시 컴포넌트를 찾을 수 없음"));
        return;
    }

    // ECC_Visibility 채널에 대해 Block 응답 설정
    MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    LOG_Item_WARNING(TEXT("[DamageReceiver] 충돌 설정 완료: %s"), *MeshComp->GetName());
}

UMeshComponent* UDamageReceiverComponent::FindTargetMeshComponent()
{
    // 지정된 메시 컴포넌트가 있으면 사용
    if (TargetMeshComponent.IsValid())
    {
        return TargetMeshComponent.Get();
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return nullptr;
    }

    // 오너에서 메시 컴포넌트 찾기
    UMeshComponent* FoundMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!FoundMesh)
    {
        FoundMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
    }

    if (FoundMesh)
    {
        TargetMeshComponent = FoundMesh;
    }

    return FoundMesh;
}

float UDamageReceiverComponent::HandleDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // 서버에서만 데미지 처리
    if (!GetOwner()->HasAuthority())
    {
        return 0.0f;
    }

    if (CurrentHealth <= 0.0f)
    {
        return 0.0f;
    }

    // 히트 결과 정보 추출
    FHitResult HitInfo;
    bool bIsHeadshot = false;

    if (DamageEvent.GetTypeID() == FPointDamageEvent::ClassID)
    {
        const FPointDamageEvent* PointDamage = static_cast<const FPointDamageEvent*>(&DamageEvent);
        HitInfo = PointDamage->HitInfo;

        // StaticTrainingDummy인 경우 위치 기반 배율 계산
        AStaticTrainingDummy* StaticDummy = Cast<AStaticTrainingDummy>(GetOwner());
        if (StaticDummy)
        {
            FString MarkerLabel;
            FLinearColor ZoneColor;
            float FinalScore = StaticDummy->CalculateFinalScore(HitInfo.ImpactPoint, DamageAmount, MarkerLabel, ZoneColor);

            // 배율 계산 (최종점수 / 원본데미지)
            DamageMultiplier = FinalScore / DamageAmount;

            LOG_Item_WARNING(TEXT("[DamageReceiver] StaticDummy 피격 | 마커: %s | 원본 데미지: %.1f | 배율: %.2fx | 최종 데미지: %.1f"),
                *MarkerLabel, DamageAmount, DamageMultiplier, FinalScore);
        }
        // 헤드샷 판정 (스켈레탈 메시)
        else if (HitInfo.BoneName != NAME_None)
        {
            for (const FName& HeadshotBone : HeadshotBoneNames)
            {
                if (HitInfo.BoneName == HeadshotBone ||
                    HitInfo.BoneName.ToString().Contains(HeadshotBone.ToString()))
                {
                    bIsHeadshot = true;
                    DamageMultiplier = HeadshotMultiplier;
                    break;
                }
            }
        }
    }

    float FinalDamage = DamageAmount * DamageMultiplier;

    // 실제 데미지 적용
    float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - FinalDamage);
    float ActualDamage = OldHealth - CurrentHealth;

    // 델리게이트 호출
    OnDamageReceived.Broadcast(GetOwner(), ActualDamage, HitInfo, DamageCauser);

    // 체력 소진 확인
    if (CurrentHealth <= 0.0f)
    {
        HandleHealthDepleted();
    }

    return FinalDamage;
}

void UDamageReceiverComponent::HandleHealthDepleted()
{
    // 델리게이트 호출
    OnHealthDepleted.Broadcast(GetOwner());

    // 파괴 처리
    if (bDestroyOnHealthDepleted)
    {
        if (DestructionDelay > 0.0f)
        {
            LOG_Item_WARNING(TEXT("[DamageReceiver] %s 지연 파괴 예약: %.1f초 후"),
                *GetOwner()->GetName(), DestructionDelay);
            GetWorld()->GetTimerManager().SetTimer(
                DestructionTimerHandle,
                this,
                &UDamageReceiverComponent::ExecuteDestruction,
                DestructionDelay,
                false
            );
        }
        else
        {
            LOG_Item_WARNING(TEXT("[DamageReceiver] %s 즉시 파괴 실행"), *GetOwner()->GetName());
            ExecuteDestruction();
        }
    }
    else
    {
        LOG_Item_WARNING(TEXT("[DamageReceiver] %s 파괴 비활성화됨 - 액터 유지"), *GetOwner()->GetName());
    }
}

void UDamageReceiverComponent::ExecuteDestruction()
{
    if (IsValid(GetOwner()))
    {
        LOG_Item_WARNING(TEXT("[DamageReceiver] 액터 파괴 실행: %s"), *GetOwner()->GetName());
        GetOwner()->Destroy();
    }
    else
    {
        LOG_Item_WARNING(TEXT("[DamageReceiver] 파괴하려는 액터가 이미 무효함"));
    }
}

void UDamageReceiverComponent::SetHealth(float NewHealth)
{
    if (GetOwner()->HasAuthority())
    {
        CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
    }
}

void UDamageReceiverComponent::Heal(float HealAmount)
{
    if (GetOwner()->HasAuthority() && HealAmount > 0.0f)
    {
        SetHealth(CurrentHealth + HealAmount);
    }
}

float UDamageReceiverComponent::GetHealthRatio() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

void UDamageReceiverComponent::AddGameplayTag(FGameplayTag TagToAdd)
{
    if (TagToAdd.IsValid())
    {
        OwnedGameplayTags.AddTag(TagToAdd);
    }
}

void UDamageReceiverComponent::RemoveGameplayTag(FGameplayTag TagToRemove)
{
    OwnedGameplayTags.RemoveTag(TagToRemove);
}

bool UDamageReceiverComponent::HasGameplayTag(FGameplayTag TagToCheck) const
{
    return OwnedGameplayTags.HasTag(TagToCheck);
}