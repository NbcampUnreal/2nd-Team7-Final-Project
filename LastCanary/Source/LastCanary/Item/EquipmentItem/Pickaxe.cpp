#include "Item/EquipmentItem/Pickaxe.h"
#include "Character/BaseCharacter.h"
#include "Components/SphereComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "LastCanary.h"

APickaxe::APickaxe()
{
    // 데미지 콜리전 컴포넌트 생성
    DamageCollision = CreateDefaultSubobject<USphereComponent>(TEXT("DamageCollision"));
    DamageCollision->SetSphereRadius(100.0f);
    // ⭐ 콜리전 설정 완전 수정
    DamageCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DamageCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    DamageCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

    // ⭐ 오직 Pawn만 Overlap으로 설정
    DamageCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    // ⭐ 생성할 때 bGenerateOverlapEvents를 명시적으로 true로 설정
    DamageCollision->SetGenerateOverlapEvents(true);

    // 오버랩 이벤트 바인딩
    DamageCollision->OnComponentBeginOverlap.AddDynamic(this, &APickaxe::OnDamageCollisionBeginOverlap);

    // 기본적으로는 비활성화
    DamageCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APickaxe::BeginPlay()
{
    Super::BeginPlay();

    // 메시의 소켓에 콜리전 컴포넌트 부착
    if (StaticMeshComponent && DamageCollision)
    {
        if (StaticMeshComponent->DoesSocketExist(DamageSocketName))
        {
            DamageCollision->AttachToComponent(StaticMeshComponent,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                DamageSocketName);
        }
        else
        {
            // 소켓이 없으면 메시에 직접 부착
            DamageCollision->AttachToComponent(StaticMeshComponent,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale);

            LOG_Item_WARNING(TEXT("[Pickaxe] 소켓 '%s'를 찾을 수 없어 메시에 직접 부착합니다."),
                *DamageSocketName.ToString());
        }
    }
}

void APickaxe::UseItem()
{
    // 부모 클래스의 채집 로직 실행
    Super::UseItem();

    // 서버에서만 데미지 콜리전 활성화
    if (HasAuthority())
    {
        SetDamageCollisionEnabled(true);

        // 디버그 시각화 표시
        //ShowDebugDamageSphere();

        // 일정 시간 후 콜리전 비활성화
        GetWorld()->GetTimerManager().SetTimer(CollisionTimerHandle,
            this, &APickaxe::DisableCollisionAfterDelay,
            CollisionActiveDuration, false);
    }
}

void APickaxe::OnDamageCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    LOG_Item_WARNING(TEXT("[Pickaxe] 오버랩 감지됨: %s"), OtherActor ? *OtherActor->GetName() : TEXT("None"));

    // 서버에서만 처리
    if (!HasAuthority())
    {
        return;
    }

    // 자기 자신이나 소유자는 제외
    if (OtherActor == this || OtherActor == GetOwner())
    {
        return;
    }

    // 이미 데미지를 받은 액터는 제외 (중복 데미지 방지)
    if (DamagedActors.Contains(OtherActor))
    {
        return;
    }

    // BaseCharacter만 데미지 적용
    ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(OtherActor);
    if (TargetCharacter)
    {
        Server_ApplyDamage(OtherActor);
        DamagedActors.Add(OtherActor);

        LOG_Item_WARNING(TEXT("[Pickaxe] %s에게 데미지 %f 적용"),
            *OtherActor->GetName(), DamageAmount);
    }
}

void APickaxe::Server_ApplyDamage_Implementation(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return;
    }

    ABaseCharacter* TargetCharacter = Cast<ABaseCharacter>(TargetActor);
    if (!TargetCharacter)
    {
        return;
    }

    // ⭐ UGameplayStatics::ApplyDamage 사용
    float ActualDamage = UGameplayStatics::ApplyDamage(
        TargetCharacter,                                              // Target
        DamageAmount,                                                 // BaseDamage  
        GetOwner() ? GetOwner()->GetInstigatorController() : nullptr, // EventInstigator
        this,                                                        // DamageCauser
        UDamageType::StaticClass()                                   // DamageTypeClass
    );
}

void APickaxe::SetDamageCollisionEnabled(bool bEnabled)
{
    if (DamageCollision)
    {
        if (bEnabled)
        {
            DamageCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            DamagedActors.Empty(); // 새로운 공격 시작 시 데미지받은 액터 목록 초기화
        }
        else
        {
            DamageCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
}

void APickaxe::DisableCollisionAfterDelay()
{
    SetDamageCollisionEnabled(false);
    GetWorld()->GetTimerManager().ClearTimer(CollisionTimerHandle);
}

void APickaxe::ShowDebugDamageSphere()
{
    // 디버그 옵션이 꺼져있으면 표시하지 않음
    if (!bShowDamageCollisionDebug)
    {
        return;
    }

    if (DamageCollision)
    {
        // 콜리전의 현재 위치와 크기 가져오기
        FVector Location = DamageCollision->GetComponentLocation();
        float Radius = DamageCollision->GetScaledSphereRadius();
        UWorld* World = GetWorld();

        if (World)
        {
            // 빨간색 디버그 구체를 3초 동안 표시
            DrawDebugSphere(World, Location, Radius, 12, FColor::Red, false, DebugSphereDuration, 0, 2.0f);

            // 로그로 디버그 정보 출력
            LOG_Item_WARNING(TEXT("[Pickaxe] 디버그 데미지 구체 표시 - 위치: %s, 반지름: %.1f"),
                *Location.ToString(), Radius);
        }
    }
}
