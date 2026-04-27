// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UObject/NoExportTypes.h"
#include "LevelNode.generated.h"

// 关卡类型枚举
UENUM(BlueprintType)
enum class ELevelNodeType : uint8
{
    INVALID     UMETA(DisplayName = "Invalid"),       //无效，用于测试及部分初始化
    COMBAT      UMETA(DisplayName = "Combat"),        // 普通战斗
    ELITE       UMETA(DisplayName = "Elite"),         // 精英战斗
    BOSS        UMETA(DisplayName = "Boss"),          // BOSS战
    SHOP        UMETA(DisplayName = "Shop"),          // 商店
    REST        UMETA(DisplayName = "Rest"),          // 休息点
    EVENT       UMETA(DisplayName = "Event"),         // 随机事件
    TREASURE    UMETA(DisplayName = "Treasure"),      // 宝箱
    FATE        UMETA(DisplayName = "Fate")           // 命运
};

// Buff效果结构体
USTRUCT(BlueprintType)
struct FNodeBuff : public FTableRowBase
{
    GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
    FName BuffID;            // Buff唯一标识

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
    FText DisplayName;       // 显示名称

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
    FText Description;       // 描述文本

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
    float Value = 0.0f;      // 数值效果，大部分时候用不上，有可能在buff中会用到

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buff")
    int32 Duration = 0;      // 持续步数
};

USTRUCT(BlueprintType)
struct FLevelNodeSaveData//保存数据(孩子们，不要忘了SaveGame)
{
    GENERATED_BODY()
public:

    UPROPERTY(SaveGame)
    FGuid NodeGUID;

    UPROPERTY(SaveGame)
    FName NodeID;

    UPROPERTY(SaveGame)
    ELevelNodeType NodeType = ELevelNodeType::COMBAT;

    UPROPERTY(SaveGame)
    int32 FloorLevel = 0;

    UPROPERTY(SaveGame)
    int32 Column = 0;//新增节点所在的索引

    UPROPERTY(SaveGame)
    bool bIsUnlocked = false;

    UPROPERTY(SaveGame)
    bool bIsCompleted = false;

    UPROPERTY(SaveGame)
    TArray<FNodeBuff> AvailableBuffs;
};


/**
 * 
 */
UCLASS()
class AETHER_CARD_API ULevelNode : public UObject
{
	GENERATED_BODY()

public:
    ULevelNode();

    // --- 核心属性 ---（懒得一个个加函数了，直接全部暴露出去）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Node")
    int32 Column = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Node")
    FGuid NodeGUID;  // 全局唯一ID，后续可能开放联机功能，可能用于保存

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Node")
    FName NodeID;                                    // 节点唯一标识

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Node")
    ELevelNodeType NodeType = ELevelNodeType::COMBAT;// 关卡类型

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Node")
    int32 FloorLevel = 0;                            // 所在层数

    // --- Buff信息 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Node")
    TArray<FNodeBuff> AvailableBuffs;               // 可获得的Buff列表

    // --- 连接关系 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Connections")
    TArray<ULevelNode*> ParentNodes;                 // 前置节点（来源）

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Connections")
    TArray<ULevelNode*> ChildNodes;                  // 后置节点（去向）

    // --- 状态信息 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsUnlocked = false;                        // 是否已解锁

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsCompleted = false;                       // 是否已完成

    // --- 资源引用 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    FSoftObjectPath LevelAssetPath;                  // 关卡场景路径

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TSoftObjectPtr<UTexture2D> PreviewImage;        // 预览图

    // --- 功能方法（面对对象） ---
    UFUNCTION(BlueprintCallable, Category = "Level Node")
    void AddParentNode(ULevelNode* ParentNode);//加入父节点

    UFUNCTION(BlueprintCallable, Category = "Level Node")
    void AddChildNode(ULevelNode* ChildNode);//加入子节点

    UFUNCTION(BlueprintCallable, Category = "Level Node")
    void SetUnlocked(bool bUnlocked);//特殊情况下直接解锁，开局调用

    UFUNCTION(BlueprintCallable, Category = "Level Node")
    void SetCompleted(bool bCompleted);//结束调用

    UFUNCTION(BlueprintCallable, Category = "Level Node")
    bool IsReachable() const;                       // 是否可达（有解锁的父节点）

    UFUNCTION(BlueprintCallable, Category = "Level Node")
    void AddBuff(const FNodeBuff& NewBuff);//加入buff

    UFUNCTION(BlueprintCallable, Category = "Level Node")
    void ClearBuffs();//消除buff
	
};
