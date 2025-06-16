#include "Item/EquipmentItem/WalkieTalkie.h"
#include "WalkieTalkie.h"
#include "LastCanary.h"

void AWalkieTalkie::UseItem()
{
	Super::UseItem();

	StartWalkieTalkie();
	LOG_Item_WARNING(TEXT("WTF"));
}
