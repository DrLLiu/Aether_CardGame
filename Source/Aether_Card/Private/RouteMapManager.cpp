#include "RouteMapManager.h"
#include "MySaveGame.h"  // 假定项目中已实现 UMySaveGame


URouteMapManager::URouteMapManager()
{
	// 设置默认权重（示例值，可后续在蓝图或编辑器中修改）
	NodeTypeWeights.Add(ELevelNodeType::COMBAT, 50.0f);
	NodeTypeWeights.Add(ELevelNodeType::ELITE, 15.0f);
	NodeTypeWeights.Add(ELevelNodeType::SHOP, 15.0f);
	NodeTypeWeights.Add(ELevelNodeType::REST, 15.0f);
	NodeTypeWeights.Add(ELevelNodeType::EVENT, 10.0f);
	NodeTypeWeights.Add(ELevelNodeType::TREASURE, 0.0f);
	NodeTypeWeights.Add(ELevelNodeType::FATE, 0.0f);
}

void URouteMapManager::GenerateAllLayers()
{
	ClearAllMaps();

	MapLayers.SetNum(NumLayers);
	for (int32 i = 0; i < NumLayers; ++i)
	{
		GenerateLayer(i);
	}
}

void URouteMapManager::ClearAllMaps()
{
	AllGeneratedNodes.Empty();
	MapLayers.Empty();
}

FRouteMapLayer& URouteMapManager::GetMapLayer(int32 LayerIndex)
{
	check(MapLayers.IsValidIndex(LayerIndex));
	return MapLayers[LayerIndex];
}

void URouteMapManager::GenerateLayer(int32 LayerIndex)
{
	FRouteMapLayer Layer;

	// 1. 创建起点节点（列 0，行 0）
	ULevelNode* StartNode = NewObject<ULevelNode>(this);
	StartNode->NodeType = ELevelNodeType::COMBAT; // 起点可视为特殊战斗或自定义
	StartNode->FloorLevel = LayerIndex + 1;       // 楼层从 1 开始计数
	Layer.StartNode = StartNode;
	Layer.Positions.Add(StartNode, FIntPoint(0, 0));

	// 2. 创建中间列节点
	const int32 TotalColumns = NumColumnsPerLayer + 2; // 起点 + 中间 + Boss
	TArray<TArray<ULevelNode*>> Columns;
	Columns.SetNum(TotalColumns);
	Columns[0].Add(StartNode); // 第 0 列

	for (int32 col = 1; col <= NumColumnsPerLayer; ++col)
	{
		const int32 NodeCount = FMath::RandRange(NumNodesPerColumnRange.X, NumNodesPerColumnRange.Y);
		TArray<ULevelNode*>& ColNodes = Columns[col];
		for (int32 row = 0; row < NodeCount; ++row)
		{
			ULevelNode* Node = NewObject<ULevelNode>(this);
			Node->NodeType = SelectNodeType();
			Node->FloorLevel = LayerIndex + 1;
			ColNodes.Add(Node);
			Layer.Positions.Add(Node, FIntPoint(col, row));
		}
	}

	// 3. 创建 Boss 节点（最后一列，行 0）
	ULevelNode* BossNode = NewObject<ULevelNode>(this);
	BossNode->NodeType = ELevelNodeType::BOSS;
	BossNode->FloorLevel = LayerIndex + 1;
	Columns.Last().Add(BossNode);
	Layer.BossNode = BossNode;
	Layer.Positions.Add(BossNode, FIntPoint(TotalColumns - 1, 0));

	// 4. 收集所有节点到 Layer.Nodes
	for (int32 col = 0; col < TotalColumns; ++col)
	{
		Layer.Nodes.Append(Columns[col]);
	}

	// 5. 建立连接（保证无交叉）
	// 起点 -> 第一列所有节点
	if (Columns[1].Num() > 0)
	{
		for (ULevelNode* Child : Columns[1])
		{
			StartNode->AddChildNode(Child);
		}
	}

	// 中间列逐列连接（无交叉算法）
	for (int32 col = 1; col < NumColumnsPerLayer; ++col)
	{
		ConnectColumns(Columns[col], Columns[col + 1]);
	}

	// 最后一列 -> Boss
	const int32 LastMiddleCol = NumColumnsPerLayer;
	if (Columns[LastMiddleCol].Num() > 0)
	{
		for (ULevelNode* Parent : Columns[LastMiddleCol])
		{
			Parent->AddChildNode(BossNode);
		}
	}

	MapLayers[LayerIndex] = MoveTemp(Layer);
	AllGeneratedNodes.Append(MapLayers[LayerIndex].Nodes);
}

ELevelNodeType URouteMapManager::SelectNodeType() const
{
	float TotalWeight = 0.0f;
	for (const auto& Pair : NodeTypeWeights)
	{
		TotalWeight += Pair.Value;
	}

	if (TotalWeight <= 0.0f)
	{
		return ELevelNodeType::COMBAT; // 保底
	}

	float Random = FMath::FRandRange(0.0f, TotalWeight);
	for (const auto& Pair : NodeTypeWeights)
	{
		Random -= Pair.Value;
		if (Random <= 0.0f)
		{
			return Pair.Key;
		}
	}
	return ELevelNodeType::COMBAT;
}

void URouteMapManager::ConnectColumns(const TArray<ULevelNode*>& ColA, const TArray<ULevelNode*>& ColB)
{
	const int32 N = ColA.Num();
	const int32 M = ColB.Num();
	if (N == 0 || M == 0) return;
	// 每个A节点分配一个“主索引”，后续将据此划分连续且递增的区间
	TArray<int32> PrimaryIndices;
	PrimaryIndices.Reserve(N);
	if (N >= M)
	{
		// A比B多：每个B至少被一个A连接，多余的A可以随机指向某个B
		// 均匀分配主索引，并强制首尾为0和M-1
		for (int32 i = 0; i < N; ++i)
		{
			int32 Index = (N == 1) ? 0 : FMath::RoundToInt((float)i * (M - 1) / (N - 1));
			PrimaryIndices.Add(FMath::Clamp(Index, 0, M - 1));
		}
		PrimaryIndices[0] = 0;
		PrimaryIndices.Last() = M - 1;
		PrimaryIndices.Sort();
	}
	else // N < M
	{
		// 特殊处理 N == 1：唯一节点直接连所有B
		if (N == 1)
		{
			for (int32 j = 0; j < M; ++j)
			{
				ColA[0]->AddChildNode(ColB[j]);
			}
			return;
		}
		// 普通情况：选N个索引，必须包含0和M-1
		PrimaryIndices.Add(0);
		// 从 1..M-2 中随机选出 N-2 个不重复索引
		TArray<int32> MiddlePool;
		for (int32 j = 1; j < M - 1; ++j) MiddlePool.Add(j);
		const int32 Needed = N - 2;                     // 需要选取的数量
		const int32 Available = MiddlePool.Num();       // 可用中间索引数
		const int32 ToPick = FMath::Min(Needed, Available); // 实际可选取数（>=0）
		// 用Fisher‑Yates部分洗牌来随机挑选（保证不重复）
		for (int32 k = 0; k < ToPick; ++k)
		{
			int32 RandIdx = FMath::RandRange(k, Available - 1);
			MiddlePool.Swap(k, RandIdx);
		}
		// 取前ToPick个加入主索引
		for (int32 i = 0; i < ToPick; ++i)
		{
			PrimaryIndices.Add(MiddlePool[i]);
		}
		PrimaryIndices.Add(M - 1);
		PrimaryIndices.Sort();
		// 确保长度为N（极端情况下，比如 N==2 且 M==2，ToPick=0，长度正好为2）
		// 若因为数值问题长度仍不足N（理论上不会），用最后一个索引补齐
		while (PrimaryIndices.Num() < N)
		{
			PrimaryIndices.Add(M - 1);
		}
		// 若超出（概率为0），削减尾部
		PrimaryIndices.SetNum(N, false);
	}
	// 划分区间：左边界紧接上一区间的右边界+1，保证连续无重叠
	struct FRange { int32 L; int32 R; };
	TArray<FRange> Ranges;
	Ranges.SetNum(N);
	int32 CurrentL = 0;
	for (int32 i = 0; i < N; ++i)
	{
		Ranges[i].L = CurrentL;
		Ranges[i].R = PrimaryIndices[i];
		// 防止出现 L > R 的无效区间（发生原因：主索引未严格递增）
		if (Ranges[i].L > Ranges[i].R)
		{
			Ranges[i].L = Ranges[i].R = FMath::Clamp(PrimaryIndices[i], 0, M - 1);
		}
		CurrentL = Ranges[i].R + 1;
	}
	// 最后一个区间的右边界必须覆盖到 M-1
	Ranges.Last().R = M - 1;
	// 实际建立连接
	for (int32 i = 0; i < N; ++i)
	{
		const int32 Start = FMath::Clamp(Ranges[i].L, 0, M - 1);
		const int32 End = FMath::Clamp(Ranges[i].R, 0, M - 1);
		for (int32 j = Start; j <= End; ++j)
		{
			ColA[i]->AddChildNode(ColB[j]);
		}
	}
}

// ---------- 保存与加载 ----------

void URouteMapManager::SaveToSaveGame(UMySaveGame* SaveGame) const//内部保存函数
{
	if (!SaveGame) return;

	// 假设 UMySaveGame 包含以下字段：
	// TArray<FLevelNodeSaveData> SavedNodes;
	// TArray<FNodeConnectionSaveData> SavedConnections;
	// TArray<FNodePositionSaveData> SavedPositions;

	SaveGame->SavedNodes.Empty();
	SaveGame->SavedConnections.Empty();
	SaveGame->SavedPositions.Empty();

	// 序列化所有节点数据
	for (const ULevelNode* Node : AllGeneratedNodes)
	{
		FLevelNodeSaveData Data;
		Data.NodeGUID = Node->NodeGUID;
		Data.NodeID = Node->NodeID;
		Data.NodeType = Node->NodeType;
		Data.FloorLevel = Node->FloorLevel;
		Data.bIsUnlocked = Node->bIsUnlocked;
		Data.bIsCompleted = Node->bIsCompleted;
		Data.AvailableBuffs = Node->AvailableBuffs;
		SaveGame->SavedNodes.Add(Data);

		// 查找节点位置
		for (const FRouteMapLayer& Layer : MapLayers)
		{
			const FIntPoint* Pos = Layer.Positions.Find(Node);
			if (Pos)
			{
				FNodePositionSaveData PosData;
				PosData.NodeGUID = Node->NodeGUID;
				PosData.Column = Pos->X;
				PosData.Row = Pos->Y;
				SaveGame->SavedPositions.Add(PosData);
				break;
			}
		}
	}

	// 序列化连接关系
	for (const ULevelNode* Node : AllGeneratedNodes)
	{
		for (const ULevelNode* Child : Node->ChildNodes)
		{
			FNodeConnectionSaveData ConnData;
			ConnData.FromNodeGUID = Node->NodeGUID;
			ConnData.ToNodeGUID = Child->NodeGUID;
			SaveGame->SavedConnections.Add(ConnData);
		}
	}
}

UMySaveGame* URouteMapManager::CreateSaveGameData() const//提供给GameInstance的保存
{
	// 创建一个全新的保存对象
	UMySaveGame* SaveObj = NewObject<UMySaveGame>();
	if (SaveObj)
	{
		// 直接复用已有的保存逻辑
		SaveToSaveGame(SaveObj);
	}
	return SaveObj;
}

void URouteMapManager::LoadFromSaveGame(UMySaveGame* SaveGame)
{
	if (!SaveGame) return;

	ClearAllMaps();

	TMap<FGuid, ULevelNode*> GuidMap;

	// 重建所有节点
	for (const FLevelNodeSaveData& Data : SaveGame->SavedNodes)
	{
		ULevelNode* Node = NewObject<ULevelNode>(this);
		Node->NodeGUID = Data.NodeGUID;
		Node->NodeID = Data.NodeID;
		Node->NodeType = Data.NodeType;
		Node->FloorLevel = Data.FloorLevel;
		Node->bIsUnlocked = Data.bIsUnlocked;
		Node->bIsCompleted = Data.bIsCompleted;
		Node->AvailableBuffs = Data.AvailableBuffs;
		GuidMap.Add(Data.NodeGUID, Node);
	}

	// 重建连接
	for (const FNodeConnectionSaveData& Conn : SaveGame->SavedConnections)
	{
		ULevelNode* From = GuidMap.FindRef(Conn.FromNodeGUID);
		ULevelNode* To = GuidMap.FindRef(Conn.ToNodeGUID);
		if (From && To)
		{
			From->AddChildNode(To);
		}
	}

	// 按楼层归入各层，并恢复位置映射
	MapLayers.SetNum(NumLayers);

	for (const auto& Pair : GuidMap)
	{
		ULevelNode* Node = Pair.Value;
		const int32 LayerIdx = Node->FloorLevel - 1; // 假设楼层从 1 开始
		if (LayerIdx >= 0 && LayerIdx < NumLayers)
		{
			MapLayers[LayerIdx].Nodes.Add(Node);
		}
	}

	for (const FNodePositionSaveData& Pos : SaveGame->SavedPositions)
	{
		ULevelNode* Node = GuidMap.FindRef(Pos.NodeGUID);
		if (!Node) continue;

		for (FRouteMapLayer& Layer : MapLayers)
		{
			if (Layer.Nodes.Contains(Node))
			{
				Layer.Positions.Add(Node, FIntPoint(Pos.Column, Pos.Row));

				if (Pos.Column == 0)
					Layer.StartNode = Node;
				else if (Pos.Column == NumColumnsPerLayer + 1)
					Layer.BossNode = Node;
				break;
			}
		}
	}

	// 更新全局节点引用
	AllGeneratedNodes.Empty();
	for (const auto& Pair : GuidMap)
	{
		AllGeneratedNodes.Add(Pair.Value);
	}
}