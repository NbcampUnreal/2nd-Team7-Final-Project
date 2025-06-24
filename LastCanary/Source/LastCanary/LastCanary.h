// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// 공통 로그 위치
#define LOG_CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(") ") + *GetNameSafe(this))

//로그 활성화 설정 (0->비활성화 / 1->활성화)
#define ENABLE_AI_LOGGING     1
#define ENABLE_SERVER_LOGGING 1
#define ENABLE_CHAR_LOGGING   1
#define ENABLE_ART_LOGGING    1
#define ENABLE_ITEM_LOGGING   1
#define ENABLE_FRAME_LOGGING  1
#define ENABLE_GAME_LOGGING   1

#pragma region LogCategoryDeclaration

DECLARE_LOG_CATEGORY_EXTERN(AI_LOG, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(Server_LOG, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(Char_LOG, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(Art_LOG, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(Item_LOG, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(Frame_LOG, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(Game_LOG, Log, All);
#pragma endregion


#pragma region AI

#if ENABLE_AI_LOGGING
#define LOG_AI(Verbosity, Format, ...) UE_LOG(AI_LOG, Verbosity, TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_AI_WARNING(Format, ...)    UE_LOG(AI_LOG, Warning,  TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_AI_ERROR(Format, ...)      UE_LOG(AI_LOG, Error,    TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define CHECK_AI(Expr, RetVal)         { if (!(Expr)) { LOG_AI_ERROR(TEXT("CHECK FAILED: %s"), TEXT(#Expr)); return RetVal; } }
#else
#define LOG_AI(...)
#define LOG_AI_WARNING(...)
#define LOG_AI_ERROR(...)
#define CHECK_AI(Expr, RetVal)
#endif

#pragma endregion


#pragma region Server

#if ENABLE_SERVER_LOGGING
#define LOG_Server(Verbosity, Format, ...) UE_LOG(Server_LOG, Verbosity, TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_Server_WARNING(Format, ...)    UE_LOG(Server_LOG, Warning,  TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_Server_ERROR(Format, ...)      UE_LOG(Server_LOG, Error,    TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define CHECK_Server(Expr, RetVal)         { if (!(Expr)) { LOG_Server_ERROR(TEXT("CHECK FAILED: %s"), TEXT(#Expr)); return RetVal; } }
#else
#define LOG_Server(...)
#define LOG_Server_WARNING(...)
#define LOG_Server_ERROR(...)
#define CHECK_Server(Expr, RetVal)
#endif

#pragma endregion


#pragma region Character

#if ENABLE_CHAR_LOGGING
#define LOG_Char(Verbosity, Format, ...) UE_LOG(Char_LOG, Verbosity, TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_Char_WARNING(Format, ...)    UE_LOG(Char_LOG, Warning,  TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_Char_ERROR(Format, ...)      UE_LOG(Char_LOG, Error,    TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define CHECK_Char(Expr, RetVal)         { if (!(Expr)) { LOG_Char_ERROR(TEXT("CHECK FAILED: %s"), TEXT(#Expr)); return RetVal; } }
#else
#define LOG_Char(...)
#define LOG_Char_WARNING(...)
#define LOG_Char_ERROR(...)
#define CHECK_Char(Expr, RetVal)
#endif

#pragma endregion


#pragma region Art

#if ENABLE_ART_LOGGING
#define LOG_Art(Verbosity, Format, ...) UE_LOG(Art_LOG, Verbosity, TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_Art_WARNING(Format, ...)    UE_LOG(Art_LOG, Warning,  TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_Art_ERROR(Format, ...)      UE_LOG(Art_LOG, Error,    TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define CHECK_Art(Expr, RetVal)         { if (!(Expr)) { LOG_Art_ERROR(TEXT("CHECK FAILED: %s"), TEXT(#Expr)); return RetVal; } }
#else
#define LOG_Art(...)
#define LOG_Art_WARNING(...)
#define LOG_Art_ERROR(...)
#define CHECK_Art(Expr, RetVal)
#endif

#pragma endregion


#pragma region Item

#if ENABLE_ITEM_LOGGING
#define LOG_Item(Verbosity, Format, ...) UE_LOG(Item_LOG, Verbosity, TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_Item_WARNING(Format, ...)    UE_LOG(Item_LOG, Warning,  TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_Item_ERROR(Format, ...)      UE_LOG(Item_LOG, Error,    TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define CHECK_Item(Expr, RetVal)         { if (!(Expr)) { LOG_Item_ERROR(TEXT("CHECK FAILED: %s"), TEXT(#Expr)); return RetVal; } }
#else
#define LOG_Item(...)
#define LOG_Item_WARNING(...)
#define LOG_Item_ERROR(...)
#define CHECK_Item(Expr, RetVal)
#endif

#pragma endregion


#pragma region FrameWork

#if ENABLE_FRAME_LOGGING
#define LOG_Frame(Verbosity, Format, ...) UE_LOG(Frame_LOG, Verbosity, TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_Frame_WARNING(Format, ...)    UE_LOG(Frame_LOG, Warning,  TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_Frame_ERROR(Format, ...)      UE_LOG(Frame_LOG, Error,    TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define CHECK_Frame(Expr, RetVal)         { if (!(Expr)) { LOG_Frame_ERROR(TEXT("CHECK FAILED: %s"), TEXT(#Expr)); return RetVal; } }
#else
#define LOG_Frame(...)
#define LOG_Frame_WARNING(...)
#define LOG_Frame_ERROR(...)
#define CHECK_Frame(Expr, RetVal)
#endif

#pragma endregion

#pragma region Game

#if ENABLE_GAME_LOGGING
#define LOG_Game(Verbosity, Format, ...) UE_LOG(Game_LOG, Verbosity, TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_Game_WARNING(Format, ...)    UE_LOG(Game_LOG, Warning,  TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define LOG_Game_ERROR(Format, ...)      UE_LOG(Game_LOG, Error,    TEXT("%s %s"), *LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define CHECK_Game(Expr, RetVal)         { if (!(Expr)) { LOG_Game_ERROR(TEXT("CHECK FAILED: %s"), TEXT(#Expr)); return RetVal; } }
#else
#define LOG_Game(...)
#define LOG_Game_WARNING(...)
#define LOG_Game_ERROR(...)
#define CHECK_Game(Expr, RetVal)
#endif

#pragma endregion