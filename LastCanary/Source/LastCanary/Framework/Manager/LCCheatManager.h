#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "LCCheatManager.generated.h"

/**
 *
 */

DECLARE_LOG_CATEGORY_EXTERN(LogCheat, Log, Warning);

UCLASS()
class LASTCANARY_API ULCCheatManager : public UCheatManager
{
	GENERATED_BODY()

protected:
	virtual void InitCheatManager() override;

public:
	/** 자원 아이템을 전부 지급 */
	UFUNCTION(exec)
	void GiveAllResources();

	/** 체력 회복 */
	UFUNCTION(exec)
	void Heal();

	/** 데미지 주기 */
	UFUNCTION(exec)
	void DamageSelf(int32 Amount);

	// 플레이어 속도 설정
	UFUNCTION(exec)
	void SetPlayerSpeed(float NewSpeed);

	// 골드 지급
	UFUNCTION(exec)
	void AddGold(int32 Amount);

	// 모든 AI 제거
	UFUNCTION(exec)
	void KillAllEnemies();

	// 아이템 생성
	UFUNCTION(exec)
	void SpawnItem(FName ItemRowName);

	/** 무적모드 전환 */
	UFUNCTION(exec)
	void ToggleGodMode();

	/** 무한 달리기*/
	UFUNCTION(exec)
	void RunningMan();

	/** 현재 위치 출력 */
	UFUNCTION(exec)
	void PrintLocation();

	/** 맵 이동 */
	UFUNCTION(exec)
	void TravelToMap(FName MapName);

	/** 주요 프레임워크 출력*/
	UFUNCTION(Exec)
	void ShowPlayerFrameworkInfo();

	/** 현재 무적 여부 확인 */
	bool IsGodModeEnabled() const { return bGodMode; }

private:
	/** 무적 모드 상태 */
	bool bGodMode = false;
};
