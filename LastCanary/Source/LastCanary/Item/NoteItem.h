#pragma once

#include "CoreMinimal.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "DataTable/ItemDataRow.h"
#include "NoteItem.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ANoteItem : public AEquipmentItemBase
{
	GENERATED_BODY()
	
public:
	virtual void UseItem() override;

	/** 고정된 노트 이미지 인덱스 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Note")
	int32 SelectedNoteImageIndex = -1;

protected:
	/** 노트 이미지 인덱스 최초 선택 */
	void InitializeNoteImageIndex();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
