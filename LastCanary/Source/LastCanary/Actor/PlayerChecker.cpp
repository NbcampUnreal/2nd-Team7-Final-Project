#include "Actor/PlayerChecker.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "Framework/GameMode/LCGameMode.h"
#include "DataType/SessionPlayerInfo.h"

// Sets default values
APlayerChecker::APlayerChecker()
{
	PrimaryActorTick.bCanEverTick = false;

    // BoxComponent 생성 및 설정
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerVolume->SetBoxExtent(FVector(200.f, 200.f, 100.f)); // 예시 크기
    RootComponent = TriggerVolume;

}

// Called when the game starts or when spawned
void APlayerChecker::BeginPlay()
{
	Super::BeginPlay();

    if (TriggerVolume)
    {
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APlayerChecker::OnOverlapBegin);
        TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &APlayerChecker::OnOverlapEnd);
    }
}

void APlayerChecker::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor) return;

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
    if (!OtherActor) return;

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

