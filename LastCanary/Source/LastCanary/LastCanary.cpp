// Copyright Epic Games, Inc. All Rights Reserved.

#include "LastCanary.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, LastCanary, "LastCanary" );

#pragma region GeneralLogging

DEFINE_LOG_CATEGORY(AI_LOG);
DEFINE_LOG_CATEGORY(Server_LOG);
DEFINE_LOG_CATEGORY(Char_LOG);
DEFINE_LOG_CATEGORY(Art_LOG);
DEFINE_LOG_CATEGORY(Item_LOG);
DEFINE_LOG_CATEGORY(Frame_LOG);
DEFINE_LOG_CATEGORY(Game_LOG);

#pragma endregion