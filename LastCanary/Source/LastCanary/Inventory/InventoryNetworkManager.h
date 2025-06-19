#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InventoryNetworkManager.generated.h"

class UInventoryComponentBase;

/**
 * 인벤토리 관련 네트워크 처리를 담당하는 매니저
 */
UCLASS()
class LASTCANARY_API UInventoryNetworkManager : public UObject
{
	GENERATED_BODY()
	
public:
    UInventoryNetworkManager();

    /** 네트워크 핸들러 초기화 */
    UFUNCTION(BlueprintCallable, Category = "Inventory Network")
    void Initialize(UInventoryComponentBase* InOwnerInventory);

    //-----------------------------------------------------
    // RPC 패턴 표준화
    //-----------------------------------------------------

    /** 클라이언트에서 서버로 함수 호출 요청 */
    template<typename T, typename... Args>
    void CallServerFunction(T* Component, void(T::* ServerFunc)(Args...), Args... Arguments);

    /** 서버에서 모든 클라이언트로 함수 호출 */
    template<typename T, typename... Args>
    void CallMulticastFunction(T* Component, void(T::* MulticastFunc)(Args...), Args... Arguments);

    /** 서버에서 특정 클라이언트로 함수 호출 */
    template<typename T, typename... Args>
    void CallClientFunction(T* Component, APlayerController* TargetPC, void(T::* ClientFunc)(Args...), Args... Arguments);

    //-----------------------------------------------------
    // 공통 네트워크 검증
    //-----------------------------------------------------

    /** 서버 권한 확인 */
    UFUNCTION(BlueprintPure, Category = "Inventory Network")
    bool HasServerAuthority() const;

    /** 로컬 컨트롤러 확인 */
    UFUNCTION(BlueprintPure, Category = "Inventory Network")
    bool IsLocalController() const;

    /** 네트워크 환경 확인 */
    UFUNCTION(BlueprintPure, Category = "Inventory Network")
    FString GetNetworkRole() const;

private:
    /** 소유자 인벤토리 참조 */
    UPROPERTY()
    UInventoryComponentBase* OwnerInventory;

    /** 네트워크 상태 로깅 */
    void LogNetworkCall(const FString& FunctionName, const FString& Direction) const;
};
