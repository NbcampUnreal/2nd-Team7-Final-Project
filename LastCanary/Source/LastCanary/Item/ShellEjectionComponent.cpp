#include "Item/ShellEjectionComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LastCanary.h"

UShellEjectionComponent::UShellEjectionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UShellEjectionComponent::BeginPlay()
{
    Super::BeginPlay();

    // 스켈레탈 메시와 소켓 캐싱
    CacheSkeletalMeshAndSocket();
}

void UShellEjectionComponent::CacheSkeletalMeshAndSocket()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        LOG_Item_WARNING(TEXT("[ShellEjectionComponent] Owner가 없습니다."));
        return;
    }

    // 총기의 스켈레탈 메시 컴포넌트 찾기
    CachedSkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();

    if (!CachedSkeletalMesh)
    {
        LOG_Item_WARNING(TEXT("[ShellEjectionComponent] %s에서 SkeletalMeshComponent를 찾을 수 없습니다."), *Owner->GetName());
        return;
    }

    // 우선순위에 따라 유효한 소켓 찾기
    if (CachedSkeletalMesh->DoesSocketExist(ShellEjectionSocketName))
    {
        ValidSocketName = ShellEjectionSocketName;
    }
    else
    {
        // 대체 소켓들 확인
        for (const FName& FallbackSocket : FallbackSocketNames)
        {
            if (CachedSkeletalMesh->DoesSocketExist(FallbackSocket))
            {
                ValidSocketName = FallbackSocket;
                LOG_Item_WARNING(TEXT("[ShellEjectionComponent] 대체 소켓 '%s' 사용"), *FallbackSocket.ToString());
                break;
            }
        }

        if (ValidSocketName.IsNone())
        {
            LOG_Item_WARNING(TEXT("[ShellEjectionComponent] 유효한 탄피 배출 소켓을 찾을 수 없습니다. 기본 위치를 사용합니다."));
        }
    }
}

void UShellEjectionComponent::EjectShell()
{
    // 배출 가능 여부 확인
    if (!ShouldEjectShell())
    {
        return;
    }

    // 거리 확인 (성능 최적화)
    if (!IsWithinEffectDistance())
    {
        return;
    }

    // 탄피 배출 위치와 회전 계산
    FVector EjectionLocation;
    FRotator EjectionRotation;

    if (!GetEjectionTransform(EjectionLocation, EjectionRotation))
    {
        LOG_Item_WARNING(TEXT("[ShellEjectionComponent] 탄피 배출 위치를 계산할 수 없습니다."));
        return;
    }

    // 파티클 이펙트 스폰
    SpawnShellParticleEffect(EjectionLocation, EjectionRotation);

    // 사운드 재생
    PlayShellEjectionSound(EjectionLocation);

    // 쿨다운 시간 업데이트
    LastEjectionTime = GetWorld()->GetTimeSeconds();
}

bool UShellEjectionComponent::ShouldEjectShell() const
{
    // 파티클 시스템이 설정되어 있는지 확인
    if (!ShellParticleSystem)
    {
        LOG_Item_WARNING(TEXT("[ShellEjectionComponent] ShellParticleSystem이 설정되지 않았습니다."));
        return false;
    }

    // 쿨다운 확인
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastEjectionTime < MinEjectionInterval)
    {
        LOG_Item_WARNING(TEXT("[ShellEjectionComponent] 쿨다운 중입니다. (%.3fs 남음)"), MinEjectionInterval - (CurrentTime - LastEjectionTime));
        return false;
    }

    return true;
}

bool UShellEjectionComponent::GetEjectionTransform(FVector& OutLocation, FRotator& OutRotation) const
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    // 캐시된 스켈레탈 메시와 소켓 사용
    if (CachedSkeletalMesh && !ValidSocketName.IsNone())
    {
        // 소켓에서 위치와 회전 가져오기
        FTransform SocketTransform = CachedSkeletalMesh->GetSocketTransform(ValidSocketName, RTS_World);

        OutLocation = SocketTransform.GetLocation();
        OutRotation = SocketTransform.GetRotation().Rotator();

        // 방향 오프셋 적용
        FVector ForwardDirection = SocketTransform.GetRotation().GetForwardVector();
        FVector RightDirection = SocketTransform.GetRotation().GetRightVector();
        FVector UpDirection = SocketTransform.GetRotation().GetUpVector();

        OutLocation += ForwardDirection * EjectionDirectionOffset.X +
            RightDirection * EjectionDirectionOffset.Y +
            UpDirection * EjectionDirectionOffset.Z;

        return true;
    }
    else
    {
        // 소켓이 없는 경우 기본 위치 사용
        LOG_Item_WARNING(TEXT("[ShellEjectionComponent] 소켓을 사용할 수 없어 기본 위치를 사용합니다."));

        OutLocation = Owner->GetActorLocation() +
            Owner->GetActorForwardVector() * DefaultEjectionOffset.X +
            Owner->GetActorRightVector() * DefaultEjectionOffset.Y +
            Owner->GetActorUpVector() * DefaultEjectionOffset.Z;

        OutRotation = Owner->GetActorRotation();
        return true;
    }
}

bool UShellEjectionComponent::IsWithinEffectDistance() const
{
    // 로컬 플레이어 찾기
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        return true;
    }

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        return true;
    }

    // 거리 계산
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    return Distance <= MaxEffectDistance;
}

void UShellEjectionComponent::SpawnShellParticleEffect(const FVector& Location, const FRotator& Rotation)
{
    if (!ShellParticleSystem)
    {
        return;
    }

    // Niagara 파티클 시스템 스폰
    UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        ShellParticleSystem,
        Location,
        Rotation,
        FVector(1.0f), // Scale
        true, // Auto Destroy
        true, // Auto Activate
        ENCPoolMethod::None,
        true // Pre Cull Check
    );

    if (SpawnedEffect)
    {
        SpawnedEffect->SetFloatParameter(TEXT("EjectionForce"), EjectionForce);
    }
    else
    {
        LOG_Item_WARNING(TEXT("[ShellEjectionComponent] 탄피 파티클 이펙트 생성 실패"));
    }
}

void UShellEjectionComponent::PlayShellEjectionSound(const FVector& Location)
{
    if (!ShellEjectionSound)
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(
        this,
        ShellEjectionSound,
        Location,
        ShellSoundVolume
    );
}

void UShellEjectionComponent::SetShellParticleSystem(UNiagaraSystem* NewSystem)
{
    ShellParticleSystem = NewSystem;
}