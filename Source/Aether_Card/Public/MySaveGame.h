// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RouteMapManager.h" //这里面就已经包含了LevelNode中的存档数据了之前引入LevelNode出了大问题，注意循环包含这种问题
#include "GameFramework/SaveGame.h"
#include "MySaveGame.generated.h"

/**
 * 
 */
UCLASS()
class AETHER_CARD_API UMySaveGame : public USaveGame
{
	GENERATED_BODY()
public:
    //节点保存
    UPROPERTY(SaveGame)
    TArray<FLevelNodeSaveData> SavedNodes;
    UPROPERTY(SaveGame)
    TArray<FNodeConnectionSaveData> SavedConnections;
    UPROPERTY(SaveGame)
    TArray<FNodePositionSaveData> SavedPositions;
    // 当前所在楼层，存档时记录
    UPROPERTY(SaveGame)
    int32 CurrentFloorLevel = 1;

};
