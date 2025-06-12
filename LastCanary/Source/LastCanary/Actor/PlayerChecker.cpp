#include "Actor/PlayerChecker.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TimelineComponent.h"
#include "Character/BaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Framework/GameState/LCGameState.h"
#include "Framework/GameMode/LCGameMode.h"
#include "DataType/SessionPlayerInfo.h"

APlayerChecker::APlayerChecker()
{
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    // BoxComponent 생성 및 설정
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerVolume->SetBoxExtent(FVector(200.f, 200.f, 100.f)); // 예시 크기
    RootComponent = TriggerVolume;

    DoorTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DoorTimeline"));

    LeftDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftDoorMesh"));
    LeftDoorMesh->SetupAttachment(RootComponent);

    RightDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightDoorMesh"));
    RightDoorMesh->SetupAttachment(RootComponent);
}

void APlayerChecker::BeginPlay()
{
	Super::BeginPlay();

    if (TriggerVolume)
    {
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APlayerChecker::OnOverlapBegin);
        TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &APlayerChecker::OnOverlapEnd);
    }

    InitializeChecker();
}

void APlayerChecker::InitializeChecker()
{
    if (DoorOpenCurve && DoorTimeline)
    {
        DoorTimeline->Stop(); // 기존 타임라인 초기화
        DoorTimeline->SetNewTime(0.f);
        DoorTimeline->SetPlaybackPosition(0.f, false);

        FOnTimelineFloat ProgressFunction;
        ProgressFunction.BindUFunction(this, FName("HandleDoorProgress"));
        DoorTimeline->AddInterpFloat(DoorOpenCurve, ProgressFunction);

        FOnTimelineEvent TimelineFinished;
        TimelineFinished.BindUFunction(this, FName("OnTimelineFinished"));
        DoorTimeline->SetTimelineFinishedFunc(TimelineFinished);
    }

    InitialLeftRotation = LeftDoorMesh->GetRelativeRotation();
    InitialRightRotation = RightDoorMesh->GetRelativeRotation();

    bIsDoorOpen = true;

    // 직접 열기
    PlayDoorTimelineForward();
}

void APlayerChecker::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority() || OtherActor == nullptr)
    {
        return;
    }

    // Pawn인지, PlayerController가 붙어있는 Pawn인지 체크
    APawn* OverlappingPawn = Cast<APawn>(OtherActor);
    if (OverlappingPawn)
    {
        // 플레이어 컨트롤러를 가져옴
        APlayerController* PC = Cast<APlayerController>(OverlappingPawn->GetController());
        if (PC)
        {
            // PlayerState에서 이름을 가져와서 식별
            APlayerState* PS = PC->PlayerState;
            if (PS)
            {
                // 서버에서만 GameMode에 알려야 하므로
                UWorld* World = GetWorld();
                if (World && World->GetAuthGameMode())
                {
                    ALCGameMode* GM = Cast<ALCGameMode>(World->GetAuthGameMode());
                    if (GM)
                    {
                        // 준비 상태로 변경 (bIsReady = true)
                        FSessionPlayerInfo NewInfo;
                        NewInfo.PlayerName = PS->GetPlayerName();
                        NewInfo.bIsPlayerReady = true;

                        GM->SetPlayerInfo(NewInfo);
                    }
                }
            }
        }
    }
}

void APlayerChecker::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!HasAuthority() || OtherActor == nullptr)
    {
        return;
    }

    APawn* OverlappingPawn = Cast<APawn>(OtherActor);
    if (OverlappingPawn)
    {
        APlayerController* PC = Cast<APlayerController>(OverlappingPawn->GetController());
        if (PC)
        {
            APlayerState* PS = PC->PlayerState;
            if (PS)
            {
                UWorld* World = GetWorld();
                if (World && World->GetAuthGameMode())
                {
                    ALCGameMode* GM = Cast<ALCGameMode>(World->GetAuthGameMode());
                    if (GM)
                    {
                        // 준비 해제 상태로 변경 (bIsReady = false)
                        FSessionPlayerInfo NewInfo;
                        NewInfo.PlayerName = PS->GetPlayerName();
                        NewInfo.bIsPlayerReady = false;

                        GM->SetPlayerInfo(NewInfo);
                    }
                }
            }
        }
    }
}

void APlayerChecker::Server_OpenDoors_Implementation()
{
    if (bIsDoorOpen==false)
    {
        bIsDoorOpen = true;
        OnRep_DoorState();
        Multicast_OpenDoors();
    }
}

void APlayerChecker::Server_CloseDoors_Implementation()
{
    if (bIsDoorOpen)
    {
        bIsDoorOpen = false;
        OnRep_DoorState();
        Multicast_CloseDoors();
    }
}

void APlayerChecker::Multicast_OpenDoors_Implementation()
{
    PlayDoorTimelineForward();
}

void APlayerChecker::Multicast_CloseDoors_Implementation()
{
    PlayDoorTimelineReverse();
}

void APlayerChecker::PlayDoorTimelineForward()
{
    if (DoorTimeline && DoorOpenCurve)
    {
        DoorTimeline->Play();
    }
}

void APlayerChecker::PlayDoorTimelineReverse()
{
    if (DoorTimeline && DoorOpenCurve)
    {
        DoorTimeline->Reverse();
    }
}

void APlayerChecker::OnRep_DoorState()
{
    if (bIsDoorOpen)
    {
        // 클라이언트에서 들어올 때도 재생
        PlayDoorTimelineForward();
    }
    else
    {
        PlayDoorTimelineReverse();
    }
}

void APlayerChecker::HandleDoorProgress(float Value)
{
    // 문을 부드럽게 열기
    const FRotator NewLeftRotation = InitialLeftRotation + FRotator(0.f, -DoorOpenAngle * Value, 0.f);
    const FRotator NewRightRotation = InitialRightRotation + FRotator(0.f, DoorOpenAngle * Value, 0.f);

    LeftDoorMesh->SetRelativeRotation(NewLeftRotation);
    RightDoorMesh->SetRelativeRotation(NewRightRotation);
}

void APlayerChecker::OnTimelineFinished()
{
    // 열림 완료 처리 또는 사운드 추가
}

void APlayerChecker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APlayerChecker, bIsDoorOpen);
}
