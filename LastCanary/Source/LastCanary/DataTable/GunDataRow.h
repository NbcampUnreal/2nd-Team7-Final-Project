#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GunDataRow.generated.h"


UENUM(BlueprintType)
enum class EFireMode : uint8
{
    Single      UMETA(DisplayName = "단발"),
    FullAuto    UMETA(DisplayName = "연발")
};

class UNiagaraSystem;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FGunDataRow : public FTableRowBase
{
    GENERATED_BODY()

    // 기본 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Basic")
    FName GunName;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    int32 GunID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Basic")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Basic")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Basic")
    UTexture2D* IconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USkeletalMesh* GunMesh;

    // 총기 성능
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Performance")
    float BaseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Performance")
    float FireRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Performance")
    float Range;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Performance")
    float Spread;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Performance")
    int32 BulletsPerShot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Performance")
    float RecoilAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Performance")
    float ReloadTime;

    // 탄약 관련
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Ammo")
    int32 MaxAmmo;

    // 시각 효과
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Effects")
    UParticleSystem* MuzzleFlash;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Effects")
    UParticleSystem* ImpactEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Effects")
    UMaterialInterface* ImpactDecal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Effects")
    FVector DecalSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Effects")
    float DecalLifeSpan;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Effects")
    UNiagaraSystem* TrailEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Effects")
    UNiagaraSystem* ShellEjectEffect;

    // 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Sound")
    USoundBase* FireSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Sound")
    USoundBase* ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Sound")
    USoundBase* ReloadSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Sound")
    USoundBase* EmptySound;

    // 애니메이션
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Animation")
    TSubclassOf<UAnimInstance> AnimationBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Animation")
    UAnimMontage* FireAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Animation")
    UAnimMontage* ReloadAnimation;

    // 기타
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Misc")
    TArray<FName> AllowedAttachments;

    // 발사 모드 관련 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|FireMode")
    EFireMode DefaultFireMode = EFireMode::Single;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|FireMode")
    bool bCanToggleFireMode = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|FireMode")
    TArray<EFireMode> AvailableFireModes;

    // 부착물 관련 추가
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Attachments")
    bool bHasScope = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Attachments")
    UStaticMesh* ScopeMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun|Attachments")
    FName ScopeSocketName = TEXT("Scope");
};
