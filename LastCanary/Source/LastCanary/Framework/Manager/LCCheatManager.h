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
	//--------------------------------------------------------------------------------
	// ▶ 상태 조작 (체력, 속도, 무적 등)
	//--------------------------------------------------------------------------------

	/** 체력 회복 */
	UFUNCTION(exec)
	void Heal();

	/** 데미지 주기 */
	UFUNCTION(exec)
	void DamageSelf(int32 Amount);

	/** 플레이어 속도 설정 */
	UFUNCTION(exec)
	void SetPlayerSpeed(float NewSpeed);

	/** 무적모드 전환 */
	UFUNCTION(exec)
	void ToggleGodMode();

	/** 무한 달리기 */
	UFUNCTION(exec)
	void RunningMan();

	/** 현재 무적 여부 확인 */
	bool IsGodModeEnabled() const { return bGodMode; }

	//--------------------------------------------------------------------------------
	// ▶ 자원 및 아이템 관련
	//--------------------------------------------------------------------------------

	/** 자원 아이템을 전부 지급 */
	UFUNCTION(exec)
	void GiveAllResources();

	// 골드 지급
	UFUNCTION(exec)
	void AddGold(int32 Amount);

	// 아이템 생성
	UFUNCTION(exec)
	void SpawnItem(FName ItemRowName);

	/** 아이템 RowHandle로 생성 */
	UFUNCTION(exec, BlueprintCallable)
	void SpawnItemByRowHandle(FDataTableRowHandle ItemRowHandle);

	//--------------------------------------------------------------------------------
	// ▶ 적 / AI 관련
	//--------------------------------------------------------------------------------

	/** 모든 AI 제거 */
	UFUNCTION(exec)
	void KillAllEnemies();

	/** 다른 플레이어 전부 죽이기 */
	UFUNCTION(exec)
	void KillAllOthers();

	/** 몬스터 스폰 중지 */
	UFUNCTION(exec)
	void StopSpawning();

	/** 몬스터 스폰 재개 */
	UFUNCTION(exec)
	void StartSpawning();

	/** 적 소환 */
	UFUNCTION(exec)
	void SpawnEnemy(FName EnemyRowName);

	//--------------------------------------------------------------------------------
	// ▶ 위치 / 이동 / 카메라
	//--------------------------------------------------------------------------------

	/** 현재 위치 출력 */
	UFUNCTION(exec)
	void PrintLocation();

	/** 맵 이동 */
	UFUNCTION(exec)
	void TravelToMap(FName MapName);

	/** 전체 플레이어 텔레포트 */
	UFUNCTION(exec)
	void TeleportAllPlayers(float X, float Y, float Z, float Radius = 250.f);

	/** 내부 호출용 (위에서 사용) */
	UFUNCTION(exec)
	void TeleportAllPlayers_Internal(FVector CenterLocation, float OffsetRadius);


	//--------------------------------------------------------------------------------
	// ▶ 화면 효과 / 시연 연출
	//--------------------------------------------------------------------------------

	/** 밝기 강제 설정 (0 입력 시 저장된 밝기로 복원) */
	UFUNCTION(exec)
	void Lumos(float ForcedValue);

	/** 게임 일시정지/재개 토글 */
	UFUNCTION(exec)
	void PPP();

	//--------------------------------------------------------------------------------
	// ▶ 게임 흐름 제어
	//--------------------------------------------------------------------------------

	/** 베이스캠프로 탈출 */
	UFUNCTION(exec)
	void EscapeToBaseCamp();

	//--------------------------------------------------------------------------------
	// ▶ 디버그 출력
	//--------------------------------------------------------------------------------

	/** 주요 프레임워크 정보 출력 */
	UFUNCTION(exec)
	void ShowPlayerFrameworkInfo();

	/** 획득한 아이템 리스트 출력 */
	UFUNCTION(exec)
	void PrintAcquiredItems();

	//--------------------------------------------------------------------------------
	// ▶ 내부 상태
	//--------------------------------------------------------------------------------

private:

	/** 무적 모드 상태 */
	bool bGodMode = false;
};