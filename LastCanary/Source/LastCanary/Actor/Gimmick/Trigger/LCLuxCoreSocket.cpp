#include "LCLuxCoreSocket.h"
#include "LCRotationLuxStatue.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "LastCanary.h"

ALCLuxCoreSocket::ALCLuxCoreSocket()
	: bIsCoreInstalled(false)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ALCLuxCoreSocket::BeginPlay()
{
	Super::BeginPlay();
}

void ALCLuxCoreSocket::Interact_Implementation(APlayerController* Interactor)
{
	Super::Interact_Implementation(Interactor);

	if (!IsValid(TargetStatue))
	{
		LOG_Art_WARNING(TEXT("LuxCoreSocket ▶ TargetStatue가 유효하지 않음"));
		return;
	}

	// 코어 미장착 상태면 장착
	if (!bIsCoreInstalled)
	{
		InstallCore(); // 내부에서 서버/멀티 호출함
		return;
	}

	// 코어 장착된 상태면 토글 동작
	if (TargetStatue->IsLuxActive())
	{
		TargetStatue->DeactivateLux();
	}
	else
	{
		TargetStatue->ActivateLux();
	}
}

void ALCLuxCoreSocket::InstallCore()
{
	if (!HasAuthority())
	{
		Server_InstallCore();
		return;
	}

	bIsCoreInstalled = true;
	LOG_Art(Log, TEXT("LuxCoreSocket ▶ 코어 장착 완료"));

	// Target 동상 빛 켜기
	if (IsValid(TargetStatue))
	{
		TargetStatue->ActivateLux();
	}

	// 클라이언트 비주얼 처리
	Multicast_PlayInstallEffect();
}

void ALCLuxCoreSocket::Server_InstallCore_Implementation()
{
	InstallCore();
}

void ALCLuxCoreSocket::Multicast_PlayInstallEffect_Implementation()
{
	StartMovementToTarget(GetActorLocation() + FVector(0.f, 0.f, 100.f));
}

void ALCLuxCoreSocket::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALCLuxCoreSocket, bIsCoreInstalled);
	DOREPLIFETIME(ALCLuxCoreSocket, TargetStatue);
}