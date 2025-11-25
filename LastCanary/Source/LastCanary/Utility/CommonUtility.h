#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CommonUtility.generated.h"


UCLASS()
class LASTCANARY_API UCommonUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// 재시도 기반 실행
	static void RetryUntilValid(UWorld* World, TFunction<bool()> IsValidFunc, TFunction<void()> ExecuteFunc, float RetryTime = 0.1f);


    template<typename ComponentType, typename OwnerType, typename ArrayType>
    static ComponentType* CreateAndRegisterComponent(OwnerType* Owner, FName ComponentName, ArrayType& OutArray)
    {
        if (!Owner) return nullptr;

        // 컴포넌트 생성
        ComponentType* NewComp = Owner->CreateDefaultSubobject<ComponentType>(ComponentName);
        if (NewComp)
        {
            // 배열에 등록
            if (!OutArray.Contains(NewComp))
                OutArray.Add(NewComp);
        }

        return NewComp;
    }

    template<typename T>
    static T* CreateAndAttachComponent(AActor* Owner, USceneComponent* Parent, FName Name)
    {
        if (!Owner) return nullptr;

        T* NewComp = Owner->CreateDefaultSubobject<T>(Name);
        if (NewComp && Parent)
        {
            NewComp->SetupAttachment(Parent);
        }
        return NewComp;
    }
};
