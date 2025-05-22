#pragma once

#include "CoreMinimal.h"
#include "Weapon/BaseWeapon.h"
#include "Firearm.generated.h"

class AMagazine;
class ABullet;
class AParts;
class ASuppressor;
//class UUIManager;

UCLASS()
class LASTCANARY_API AFirearm : public ABaseWeapon
{
	GENERATED_BODY()
	
public:
	AFirearm();

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Bullet")
	UStaticMesh* BulletMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Component")
	USoundBase* ReloadSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Component")
	USoundBase* EmptyAmmoSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Component")
	USoundBase* ReloadFailSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Component")
	UAnimSequence* FireAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Crosshair")
	UTexture2D* CrosshairTexture;
	//UUIManager* UIManager;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void EquipParts(AParts* Parts);
	UFUNCTION()
	virtual void ReturnBulletToPool(ABullet* UsedBullet);
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void AddAmmo(int32 AmmoToAdd);
	UFUNCTION(BlueprintPure, Category = "Weapon")
	virtual int32 GetCurrentAmmoValue() const;

	//============ �α�� ���� ���� ���� ===================
	UFUNCTION(BlueprintPure, Category = "Weapon")
	virtual int32 GetMaxReloadedAmmoValue() const;
	UFUNCTION(BlueprintPure, Category = "Weapon")
	virtual int32 GetReloadedAmmoValue() const;
	//======================================================

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void DetachParts(FName SocketName);
	UFUNCTION(BlueprintPure, Category = "Weapon")
	virtual float GetFinalAccuracty() const;

	//virtual void UpdateWeaponUI() override;
	virtual void Attack() override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Reload();

	virtual void SetReloadCondition();
	virtual bool GetReloadCondition() const;


protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Component")
	UNiagaraSystem* SuppressorFireNiagara;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	USoundBase* SuppressorSound;
	UPROPERTY()
	ABullet* Bullet;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Bullet")
	TSubclassOf<ABullet> BulletClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Bullet")
	float BulletSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Bullet")
	int32 MaxAmmo;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Bullet")
	int32 CurrentAmmo;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Bullet")
	int32 MaxReloadedAmmo;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Bullet")
	int32 ReloadedAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Bullet")
	float ReloadTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Bullet", meta = (ClampMin = "0.1", ClampMax = "1.0", UIMin ="0.1", UIMax = "1.0"))
	float OriginalAccuracy;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Bullet")
	float MaxSpreadAngle;

	float FinalAccuracy;
	bool bIsLoadingComplete;
	bool bIsMagazineAttached;
	bool bIsSuppressorInstalled;
	bool bCanReload;

	FTimerHandle ReloadTimerHandle;
	TArray<ABullet*> BulletPool;
	ASuppressor* Suppressor;
	USoundBase* OriginalAttackSound;

	UFUNCTION()
	virtual void Fire();
	virtual void BeginPlay() override;

};
