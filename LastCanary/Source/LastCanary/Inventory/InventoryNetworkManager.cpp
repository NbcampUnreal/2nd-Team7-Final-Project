#include "Inventory/InventoryNetworkManager.h"
#include "Inventory/InventoryComponentBase.h"
#include "LastCanary.h"

UInventoryNetworkManager::UInventoryNetworkManager()
{
    OwnerInventory = nullptr;
}

void UInventoryNetworkManager::Initialize(UInventoryComponentBase* InOwnerInventory)
{
    OwnerInventory = InOwnerInventory;

    if (OwnerInventory)
    {
        LOG_Item_WARNING(TEXT("[InventoryNetworkHandler::Initialize] 네트워크 핸들러 초기화 완료"));
    }
}

template<typename T, typename... Args>
void UInventoryNetworkManager::CallServerFunction(T* Component, void(T::* ServerFunc)(Args...), Args... Arguments)
{
    if (!Component || !OwnerInventory)
    {
        LOG_Item_WARNING(TEXT("[CallServerFunction] Component 또는 OwnerInventory가 null"));
        return;
    }

    AActor* Owner = OwnerInventory->GetOwner();
    if (!Owner)
    {
        LOG_Item_WARNING(TEXT("[CallServerFunction] Owner가 null"));
        return;
    }

    if (Owner->HasAuthority())
    {
        LOG_Item_WARNING(TEXT("[CallServerFunction] 서버에서 직접 실행"));
        (Component->*ServerFunc)(Arguments...);
    }
    else
    {
        LogNetworkCall(TEXT("Unknown Server Function"), TEXT("Client -> Server"));
        (Component->*ServerFunc)(Arguments...);
    }
}

template<typename T, typename... Args>
void UInventoryNetworkManager::CallMulticastFunction(T* Component, void(T::* MulticastFunc)(Args...), Args... Arguments)
{
    if (!Component || !HasServerAuthority())
    {
        LOG_Item_WARNING(TEXT("[CallMulticastFunction] 서버가 아니거나 Component가 null"));
        return;
    }

    LogNetworkCall(TEXT("Unknown Multicast Function"), TEXT("Server -> All Clients"));
    (Component->*MulticastFunc)(Arguments...);
}

bool UInventoryNetworkManager::HasServerAuthority() const
{
    if (!OwnerInventory)
    {
        return false;
    }

    AActor* Owner = OwnerInventory->GetOwner();
    return Owner && Owner->HasAuthority();
}

bool UInventoryNetworkManager::IsLocalController() const
{
    if (!OwnerInventory)
    {
        return false;
    }

    AActor* Owner = OwnerInventory->GetOwner();
    if (APawn* OwnerPawn = Cast<APawn>(Owner))
    {
        return OwnerPawn->IsLocallyControlled();
    }

    return false;
}

FString UInventoryNetworkManager::GetNetworkRole() const
{
    if (!OwnerInventory)
    {
        return TEXT("Unknown");
    }

    AActor* Owner = OwnerInventory->GetOwner();
    if (!Owner)
    {
        return TEXT("No Owner");
    }

    ENetRole Role = Owner->GetLocalRole();
    switch (Role)
    {
    case ROLE_Authority: return TEXT("Server");
    case ROLE_AutonomousProxy: return TEXT("Autonomous Client");
    case ROLE_SimulatedProxy: return TEXT("Simulated Client");
    default: return TEXT("None");
    }
}

void UInventoryNetworkManager::LogNetworkCall(const FString& FunctionName, const FString& Direction) const
{
    FString NetworkRole = GetNetworkRole();
    LOG_Item_WARNING(TEXT("[Network] %s: %s (%s)"), *Direction, *FunctionName, *NetworkRole);
}
