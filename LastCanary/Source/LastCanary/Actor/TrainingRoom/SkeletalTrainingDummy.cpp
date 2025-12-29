#include "Actor/TrainingRoom/SkeletalTrainingDummy.h"
#include "Item/Component/DamageReceiverComponent.h"
#include "Actor/TrainingRoom/TrainingTargetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"


ASkeletalTrainingDummy::ASkeletalTrainingDummy()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    DummyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DummyMesh"));
    DummyMesh->SetupAttachment(RootComponent);
    DummyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DummyMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    DamageReceiverComponent = CreateDefaultSubobject<UDamageReceiverComponent>(TEXT("DamageReceiverComponent"));
    DamageReceiverComponent->MaxHealth = 100.0f;
    DamageReceiverComponent->bDestroyOnHealthDepleted = false;
    DamageReceiverComponent->bAutoAddEnemyTag = false;
    DamageReceiverComponent->HeadshotMultiplier = 2.5f;
    DamageReceiverComponent->HeadshotBoneNames.Add(FName("Head"));

    TrainingTargetComponent = CreateDefaultSubobject<UTrainingTargetComponent>(TEXT("TrainingTargetComponent"));

    GameplayTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Training.Dummy.Skeletal")));
}


void ASkeletalTrainingDummy::BeginPlay()
{
	Super::BeginPlay();
	
    if (!GameplayTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("Training.Dummy"))))
    {
        GameplayTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Training.Dummy")));
    }

    if (!GameplayTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("Character.Enemy"))))
    {
        GameplayTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Character.Enemy")));
    }

    if (DummyMesh && AnimClass && bUseAnimationBlueprint)
    {
        DummyMesh->SetAnimInstanceClass(AnimClass);
    }

    // 동적 머티리얼 생성
    if (DummyMesh && DummyMesh->GetNumMaterials() > 0)
    {
        DynamicMaterial = DummyMesh->CreateDynamicMaterialInstance(0);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), DummyColor);
        }
    }
}


void ASkeletalTrainingDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (DummyMesh && DummyMesh->GetAnimInstance())
    {
        float CurrentSpeed = GetVelocity().Size();
        DummyMesh->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
    }
}

void ASkeletalTrainingDummy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ASkeletalTrainingDummy::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    TagContainer = GameplayTags;
}

float ASkeletalTrainingDummy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // DamageReceiverComponent로 데미지 전달
    if (DamageReceiverComponent)
    {
        ActualDamage = DamageReceiverComponent->HandleDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    }

    return ActualDamage;
}

void ASkeletalTrainingDummy::OnDamageReceived(AActor* DamagedActor, float DamageAmount,
    const FHitResult& HitInfo, AActor* DamageCauser)
{
    if (!HasAuthority())
    {
        return;
    }

    LOG_Item_WARNING(TEXT("[SkeletalDummy] 데미지 받음: %.1f (남은 체력: %.1f/%.1f)"),
        DamageAmount,
        DamageReceiverComponent->GetCurrentHealth(),
        DamageReceiverComponent->GetMaxHealth());

    // 피격 효과
    if (bShowHitReaction)
    {
        Multicast_ShowHitReaction(HitInfo.ImpactPoint);
    }
}

void ASkeletalTrainingDummy::OnHealthDepleted(AActor* DamagedActor)
{
    if (!HasAuthority())
    {
        return;
    }

    LOG_Item_WARNING(TEXT("[SkeletalDummy] 체력 소진: %s"), *GetName());
}

void ASkeletalTrainingDummy::Multicast_ShowHitReaction_Implementation(FVector HitLocation)
{
    // 피격 색상 변화 효과
    if (DynamicMaterial)
    {
        // 빨간색으로 깜빡임
        DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::Red);

        // 0.2초 후 원래 색상으로
        FTimerHandle ColorResetTimer;
        GetWorld()->GetTimerManager().SetTimer(ColorResetTimer, [this]()
            {
                if (DynamicMaterial)
                {
                    DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), DummyColor);
                }
            }, 0.2f, false);
    }
}

