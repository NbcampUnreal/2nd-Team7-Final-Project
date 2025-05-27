#include "Framework/GameInstance/LCGameInstance.h"
#include "UI/Manager/LCUIManagerSettings.h"
#include "Engine/Engine.h"
#include "LastCanary.h"


void ULCGameInstance::Login()
{
    //const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    //const IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

    //const FUniqueNetIdPtr NetId = Identity->GetUniquePlayerId(0);
    //if (NetId != nullptr && Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn) return;

    //LoginDelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle
    //(
    //    0,
    //    FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::HandleLoginCompleted)
    //);

    //FString AuthType;
    //FParse::Value(FCommandLine::Get(), TEXT("AUTH_TYPE="), AuthType);

    //if (!AuthType.IsEmpty()) // 테스트용 DevAuth 계정
    //{
    //    UE_LOG(LogTemp, Log, TEXT("Logging into EOS..."));

    //    if (!Identity->AutoLogin(0))
    //    {
    //        UE_LOG(LogTemp, Warning, TEXT("Failed to login... "));
    //        Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginDelegateHandle);
    //        LoginDelegateHandle.Reset();
    //    }
    //}
    //else
    //{
    //    FOnlineAccountCredentials Credentials("AccountPortal", "", "");

    //    UE_LOG(LogTemp, Log, TEXT("Logging into EOS..."));

    //    if (!Identity->Login(0, Credentials))
    //    {
    //        UE_LOG(LogTemp, Warning, TEXT("Failed to login... "));

    //        Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginDelegateHandle);
    //        LoginDelegateHandle.Reset();
    //    }
    //}
}

void ULCGameInstance::HandleLoginCompleted(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
    //IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    //IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

    //const FUniqueNetIdPtr NetId = Identity->GetUniquePlayerId(0);
    //if (bWasSuccessful)
    //{
    //    UE_LOG(LogTemp, Log, TEXT("Login callback completed!"));
    //    UE_LOG(LogTemp, Log, TEXT("Loading cloud data and searching for a session..."));

    //    //OnProcessReturnValue.Broadcast(EPlayerEOSStateType::Login, ESessionResultType::Success);
    //    //FindSessions();
    //}
    //else
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("EOS login failed."));
    //    // 로그인 실패시 EOS 게임모드 통해 메인메뉴로 이동
    //}

    //Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginDelegateHandle);
    //LoginDelegateHandle.Reset();
}


FString ULCGameInstance::GetPlayerName() const
{
    if (IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
    {
        if (IOnlineIdentityPtr Session = Subsystem->GetIdentityInterface())
        {
            if (Session->GetLoginStatus(0) == ELoginStatus::LoggedIn)
            {
                FString Nickname = Session->GetPlayerNickname(0);
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(
                        -1,             // 키 (-1은 자동으로 지워짐)
                        5.0f,           // 지속 시간 (초)
                        FColor::Green,  // 텍스트 색상
                        FString::Printf(TEXT("NickName : %s"), *Nickname)
                    );
                }
                return Nickname;
            }
        }
    }

    return "";
}

//FString ULCGameInstance::GetPlayerName(APlayerController* PC) const
//{
//    if (!PC) return TEXT("");
//
//    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
//    if (!Subsystem) return TEXT("");
//
//    IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
//    if (!Identity.IsValid()) return TEXT("");
//
//    TSharedPtr<const FUniqueNetId> NetId = PC->PlayerState->GetUniqueId().GetUniqueNetId();
//    if (!NetId.IsValid()) return TEXT("");
//
//    return Identity->GetPlayerNickname(*NetId);
//}

bool ULCGameInstance::IsPlayerLoggedIn() const
{
    if (IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
    {
        if (IOnlineIdentityPtr Session = Subsystem->GetIdentityInterface())
        {
            if (Session->GetLoginStatus(0) == ELoginStatus::LoggedIn)
            {
                return true;
            }
        }
    }
    return false;
}


ULCUIManagerSettings* ULCGameInstance::GetUIManagerSettings() const
{
	if (UIManagerSettings)
	{
		return UIManagerSettings;
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("UIManagerSettings is not set in the GameInstance."));
		return nullptr;
	}
}

void ULCGameInstance::LoadMapData()
{
    if (MapDataTable == nullptr)
    {
       // LOG_Frame_WARNING(TEXT("MapDataTable is not assigned in USFGameInstanceSubsystem."));
        return;
    }

    static const FString ContextString(TEXT("Map Lookup"));
    TArray<FMapDataRow*> AllMaps;
    MapDataTable->GetAllRows(ContextString, AllMaps);

    for (FMapDataRow* MapDataRow : AllMaps)
    {
        if (MapDataRow)
        {
            MapDataRow->MapID = GetTypeHash(MapDataRow->MapInfo.MapName);
           // LOG_Frame_WARNING(TEXT("Loaded map: %s"), *MapDataRow->MapInfo.MapName.ToString());
        }
    }
}

void ULCGameInstance::LoadItemData()
{
    if (ItemDataTable == nullptr)
    {
       // LOG_Frame_WARNING(TEXT("ItemDataTable is not assigned in ULCGameInstance."));
        return;
    }

    static const FString ContextString(TEXT("Item Lookup"));
    TArray<FItemDataRow*> AllItems;
    ItemDataTable->GetAllRows(ContextString, AllItems);

    for (FItemDataRow* ItemDataRow : AllItems)
    {
        if (ItemDataRow)
        {
            // 예: 아이디 해시화 → 캐싱 용도
            int32 HashedID = GetTypeHash(ItemDataRow->ItemName);
            ItemDataRow->ItemID = HashedID;

            LOG_Frame_WARNING(TEXT("Loaded item: %s (Price: %d)"),
                *ItemDataRow->ItemName.ToString(),
                ItemDataRow->ItemPrice);
        }
    }

   // LOG_Frame_WARNING(TEXT("총 %d개의 상점 아이템을 로딩했습니다."), AllItems.Num());
}

void ULCGameInstance::CreateSession_Implementation(const FString& ServerName, int AmountOfSlots)
{
    UE_LOG(LogTemp, Warning, TEXT("CreateSession called in C++"));
}

void ULCGameInstance::Shutdown()
{
    Super::Shutdown();

}
