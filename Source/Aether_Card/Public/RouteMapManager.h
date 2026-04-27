#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LevelNode.h"
#include "RouteMapManager.generated.h"

// 前置声明（假设项目中已存在 UMySaveGame，且包含下述保存字段）
class UMySaveGame;

/** 用于保存节点间的连接关系 */
USTRUCT(BlueprintType)
struct FNodeConnectionSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid FromNodeGUID;

	UPROPERTY(SaveGame)
	FGuid ToNodeGUID;
};

/** 用于保存节点在网格中的位置 */
USTRUCT(BlueprintType)
struct FNodePositionSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FGuid NodeGUID;

	UPROPERTY(SaveGame)
	int32 Column = 0;

	UPROPERTY(SaveGame)
	int32 Row = 0;
};

/** 表示一层完整路线地图 */
USTRUCT(BlueprintType)
struct FRouteMapLayer
{
	GENERATED_BODY()

	// 该层所有节点（运行时使用，不直接序列化）
	UPROPERTY()
	TArray<ULevelNode*> Nodes;

	UPROPERTY()
	ULevelNode* StartNode = nullptr;

	UPROPERTY()
	ULevelNode* BossNode = nullptr;

	// 节点在网格中的位置（列，行），仅运行时维护
	TMap<ULevelNode*, FIntPoint> Positions;
};

/**
 * 路线生成管理器
 * 负责按照配置生成多层、无交叉的节点路线图，并提供序列化支持。
 */
UCLASS(BlueprintType)
class AETHER_CARD_API URouteMapManager : public UObject
{
	GENERATED_BODY()

public:
	URouteMapManager();
public:

	// ---------- 公开配置参数 ----------

	/** 普通节点类型（非起点/Boss）的出现权重，值 > 0 才会被考虑 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route Generation")
	TMap<ELevelNodeType, float> NodeTypeWeights;

	/** 中间列数（不包含起点列和 Boss 列） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route Generation")
	int32 NumColumnsPerLayer = 8;

	/** 每列节点的数量范围 [Min, Max] */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route Generation")
	FIntPoint NumNodesPerColumnRange = FIntPoint(2, 4);

	/** 总层数，默认为 3 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route Generation")
	int32 NumLayers = 3;

	// ---------- 运行时数据 ----------

	/** 所有生成的层，按索引 0..NumLayers-1 排列 */
	UPROPERTY(Transient)
	TArray<FRouteMapLayer> MapLayers;

	/** 持有所有生成的节点，防止被 GC 回收 */
	UPROPERTY(Transient)
	TArray<ULevelNode*> AllGeneratedNodes;

	// ---------- 主要接口 ----------

	/** 为所有层生成新的路线地图（会清空旧数据） */
	UFUNCTION(BlueprintCallable, Category = "Route Generation")
	void GenerateAllLayers();

	/** 清空所有地图和节点 */
	UFUNCTION(BlueprintCallable, Category = "Route Generation")
	void ClearAllMaps();

	/** 获取第 LayerIndex 层的地图引用 */
	UFUNCTION(BlueprintCallable, Category = "Route Generation")
	FRouteMapLayer& GetMapLayer(int32 LayerIndex);

	/** 将当前路线保存到 MySaveGame 对象中 */
	void SaveToSaveGame(UMySaveGame* SaveGame) const;

	/** 从 MySaveGame 对象中读取并恢复路线 */
	void LoadFromSaveGame(UMySaveGame* SaveGame);

	//提供给GameInstance的保存

	/** 创建一个填充完整路线数据的 UMySaveGame 对象（由 CardGameInstance 负责保存） */
	UFUNCTION(BlueprintCallable, Category = "Save")
	UMySaveGame* CreateSaveGameData() const;

private:
	/** 生成第 LayerIndex 层的完整地图 */
	void GenerateLayer(int32 LayerIndex);

	/** 根据 NodeTypeWeights 随机选择一个节点类型 */
	ELevelNodeType SelectNodeType() const;

	/** 在两列节点之间建立无交叉的连接（单调区间算法） */
	void ConnectColumns(const TArray<ULevelNode*>& ColA, const TArray<ULevelNode*>& ColB);
};