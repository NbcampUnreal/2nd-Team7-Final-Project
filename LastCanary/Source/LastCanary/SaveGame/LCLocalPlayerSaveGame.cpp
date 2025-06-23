#include "SaveGame/LCLocalPlayerSaveGame.h"
#include "Engine/LocalPlayer.h" // ULocalPlayer 관련
#include "Kismet/GameplayStatics.h"    // 경우에 따라 사용 가능


const TCHAR UMySaveGameUtils::PlayerSaveSlotPrefix[] = TEXT("PlayerSaveSlot_");

bool ULCLocalPlayerSaveGame::SaveMouseSensitivity(UWorld* World, float NewSensitivity)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        SaveGame->Settings.MouseSensitivity = NewSensitivity;
        return SaveGame->SaveGameToSlotForLocalPlayer();
    }
    return false;
}

float ULCLocalPlayerSaveGame::LoadMouseSensitivity(UWorld* World)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        return SaveGame->Settings.MouseSensitivity;
    }

    return DefaultSettings::DEFAULT_SENSITIVITY; // 기본값
}


bool ULCLocalPlayerSaveGame::SaveMasterVolume(UWorld* World, float NewVolume)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        SaveGame->Settings.MasterVolume = NewVolume;
        return SaveGame->SaveGameToSlotForLocalPlayer();
    }
    return false;
}

float ULCLocalPlayerSaveGame::LoadMasterVolume(UWorld* World)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        return SaveGame->Settings.MasterVolume;
    }

    return DefaultSettings::DEFAULT_VOLUME; // 기본값
}

bool ULCLocalPlayerSaveGame::SaveBGMVolume(UWorld* World, float NewVolume)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        SaveGame->Settings.BGMVolume = NewVolume;
        return SaveGame->SaveGameToSlotForLocalPlayer();
    }
    return false;
}

float ULCLocalPlayerSaveGame::LoadBGMVolume(UWorld* World)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        return SaveGame->Settings.BGMVolume;
    }

    return DefaultSettings::DEFAULT_VOLUME; // 기본값
}

bool ULCLocalPlayerSaveGame::SaveEffectVolume(UWorld* World, float NewVolume)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        SaveGame->Settings.EffectVolume = NewVolume;
        return SaveGame->SaveGameToSlotForLocalPlayer();
    }
    return false;
}

float ULCLocalPlayerSaveGame::LoadEffectVolume(UWorld* World)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        return SaveGame->Settings.EffectVolume;
    }

    return DefaultSettings::DEFAULT_VOLUME; // 기본값
}

bool ULCLocalPlayerSaveGame::SaveBrightness(UWorld* World, float NewBrightness)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        SaveGame->Settings.Brightness = NewBrightness;
        return SaveGame->SaveGameToSlotForLocalPlayer();
    }
    return false;
}

float ULCLocalPlayerSaveGame::LoadBrightness(UWorld* World)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        return SaveGame->Settings.Brightness;
    }

    return DefaultSettings::DEFAULT_BRIGHTNESS; // 기본값
}

bool ULCLocalPlayerSaveGame::SaveFullScreenMode(UWorld* World, bool NewSetting)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        SaveGame->Settings.bIsFullScreenMode = NewSetting;
        return SaveGame->SaveGameToSlotForLocalPlayer();
    }
    return false;
}

bool ULCLocalPlayerSaveGame::LoadFullScreenMode(UWorld* World)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        return SaveGame->Settings.bIsFullScreenMode;
    }

    return DefaultSettings::DEFAULT_FULLSCREEN; // 기본값
}

bool ULCLocalPlayerSaveGame::SaveKeyBindings(UWorld* World, const TArray<FSaveKeyMapping>& Mappings)
{
    ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World);
    if (!SaveGame) return false;

    SaveGame->Settings.SavedMappings = Mappings;
    return SaveGame->SaveGameToSlotForLocalPlayer();
}

TArray<FSaveKeyMapping> ULCLocalPlayerSaveGame::LoadKeyBindings(UWorld* World)
{
    ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World);
    if (!SaveGame) return TArray<FSaveKeyMapping>();

    return SaveGame->Settings.SavedMappings;
}

bool ULCLocalPlayerSaveGame::SaveMicrophoneVolume(UWorld* World, float NewVolume)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        SaveGame->Settings.MicrophoneVolume = NewVolume;
        return SaveGame->SaveGameToSlotForLocalPlayer();
    }
    return false;
}

float ULCLocalPlayerSaveGame::LoadMicrophoneVolume(UWorld* World)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        return SaveGame->Settings.MicrophoneVolume;
    }

    return DefaultSettings::DEFAULT_VOLUME; // 기본값
}


bool ULCLocalPlayerSaveGame::SaveVoiceChatVolume(UWorld* World, float NewVolume)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        SaveGame->Settings.VoiceChatVolume = NewVolume;
        return SaveGame->SaveGameToSlotForLocalPlayer();
    }
    return false;
}

float ULCLocalPlayerSaveGame::LoadVoiceChatVolume(UWorld* World)
{
    if (ULCLocalPlayerSaveGame* SaveGame = GetSaveInstance(World))
    {
        return SaveGame->Settings.VoiceChatVolume;
    }

    return DefaultSettings::DEFAULT_VOLUME; // 기본값
}

bool ULCLocalPlayerSaveGame::SaveMicrophoneMode(UWorld* World, bool NewSetting)
{
    //TODO: 보이스 기능 홀드 토글 들어오면 나중에 추가하기
    return true;
}

bool ULCLocalPlayerSaveGame::LoadMicrophoneMode(UWorld* World)
{
    //TODO: 보이스 기능 홀드 토글 들어오면 나중에 추가하기
    return true;
}

ULCLocalPlayerSaveGame* ULCLocalPlayerSaveGame::GetSaveInstance(UWorld* World)
{
    if (!World) return nullptr;

    if (UGameInstance* GameInstance = World->GetGameInstance())
    {
        if (ULocalPlayer* LocalPlayer = GameInstance->GetFirstGamePlayer())
        {
            FString SlotName = UMySaveGameUtils::MakeSlotName(LocalPlayer->GetControllerId());

            return Cast<ULCLocalPlayerSaveGame>(
                ULocalPlayerSaveGame::LoadOrCreateSaveGameForLocalPlayer(
                    ULCLocalPlayerSaveGame::StaticClass(),
                    LocalPlayer,
                    SlotName
                ));
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("ULCLocalPlayerSaveGame::GetSaveInstance - LocalPlayer not found."));
    return nullptr;
}
