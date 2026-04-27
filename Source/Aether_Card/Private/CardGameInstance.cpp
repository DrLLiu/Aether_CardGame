#include "CardGameInstance.h"
#include "RouteMapManager.h"
#include "LevelNode.h"
#include "MySaveGame.h"
#include "Kismet/GameplayStatics.h"

void UCardGameInstance::Init()
{
    // 创建路线管理器，不在此处生成地图
    RouteMapManager = NewObject<URouteMapManager>(this);
}

void UCardGameInstance::Shutdown()
{
    // 对象随 GameInstance 自动销毁
}

void UCardGameInstance::Starting()
{
    if (!RouteMapManager) return;

    // 1. 生成全部三层路线图
    RouteMapManager->GenerateAllLayers();

    // 2. 激活第一层起点
    if (RouteMapManager->MapLayers.IsValidIndex(0))
    {
        const FRouteMapLayer& FirstLayer = RouteMapManager->MapLayers[0];
        CurrentActiveNode = FirstLayer.StartNode;
        if (CurrentActiveNode)
        {
            CurrentActiveNode->SetUnlocked(true);
        }
    }

    // 3. 设置当前楼层为 1
    CurrentFloor = 1;

    // 4. 创建背景（ZOrder = 0）
    if (RouteMapBackgroundClass)
    {
        RouteMapBackground = CreateWidget<URouteMapBackground>(this, RouteMapBackgroundClass);
        if (RouteMapBackground)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Background created.")));
            RouteMapBackground->AddToViewport(0); // 最底层
            RouteMapBackground->SetFloor(CurrentFloor);

            // 绑定楼层变化委托，自动切换背景图片
            OnFloorChanged.AddDynamic(RouteMapBackground, &URouteMapBackground::OnFloorChangedHandler);
        }
    }

    // 5. 创建地图 UI（ZOrder = 1）
    if (RouteMapWidgetClass)
    {
        RouteMapWidget = CreateWidget<URouteMapWidget>(this, RouteMapWidgetClass);
        if (RouteMapWidget)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Map created.")));
            RouteMapWidget->AddToViewport(1); // 在背景之上
            RouteMapWidget->SetMaxVisibleFloor(CurrentFloor);
            RouteMapWidget->BuildMapFromLayers(RouteMapManager->MapLayers, RouteMapManager->NumColumnsPerLayer);
            RouteMapWidget->ShowMap();

            // 绑定楼层变化委托，自动更新地图楼层
            OnFloorChanged.AddDynamic(RouteMapWidget, &URouteMapWidget::OnFloorChangedHandler);

            // 让背景同步地图的可见性
            if (RouteMapBackground)
            {
                RouteMapBackground->SyncWithRouteMapWidget(RouteMapWidget);
            }
        }
    }

    // 6. 广播初始楼层（其他系统如有需要可监听）
    OnFloorChanged.Broadcast(CurrentFloor);
}

bool UCardGameInstance::SaveGame(const FString& SlotName, int32 UserIndex)
{
    if (!RouteMapManager) return false;

    // 生成存档对象（包含所有节点、连接、位置数据）
    UMySaveGame* SaveObj = RouteMapManager->CreateSaveGameData();
    if (!SaveObj) return false;

    // 写入当前楼层
    SaveObj->CurrentFloorLevel = CurrentFloor;

    return UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, UserIndex);
}

bool UCardGameInstance::LoadGame(const FString& SlotName, int32 UserIndex)
{
    if (!RouteMapManager) return false;

    // 从磁盘加载存档对象
    UMySaveGame* LoadedObj = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
    if (!LoadedObj)
    {
        UE_LOG(LogTemp, Warning, TEXT("No save found in slot: %s"), *SlotName);
        return false;
    }

    // 恢复路线数据
    RouteMapManager->LoadFromSaveGame(LoadedObj);

    // 恢复当前楼层
    CurrentFloor = LoadedObj->CurrentFloorLevel;

    // 确保背景已创建（若未创建则创建，Widget 只在第一次加载时创建）
    if (!RouteMapBackground && RouteMapBackgroundClass)
    {
        RouteMapBackground = CreateWidget<URouteMapBackground>(this, RouteMapBackgroundClass);
        if (RouteMapBackground)
        {
            RouteMapBackground->AddToViewport(0);
            OnFloorChanged.AddDynamic(RouteMapBackground, &URouteMapBackground::OnFloorChangedHandler);
        }
    }

    // 确保地图已创建
    if (!RouteMapWidget && RouteMapWidgetClass)
    {
        RouteMapWidget = CreateWidget<URouteMapWidget>(this, RouteMapWidgetClass);
        if (RouteMapWidget)
        {
            RouteMapWidget->AddToViewport(1);
            OnFloorChanged.AddDynamic(RouteMapWidget, &URouteMapWidget::OnFloorChangedHandler);
        }
    }

    // 同步当前楼层到 UI
    if (RouteMapBackground)
    {
        RouteMapBackground->SetFloor(CurrentFloor);
        if (RouteMapWidget)
        {
            RouteMapBackground->SyncWithRouteMapWidget(RouteMapWidget);
        }
    }

    if (RouteMapWidget)
    {
        RouteMapWidget->SetMaxVisibleFloor(CurrentFloor);
        RouteMapWidget->BuildMapFromLayers(RouteMapManager->MapLayers, RouteMapManager->NumColumnsPerLayer);
        RouteMapWidget->ShowMap();
    }

    // 广播楼层
    OnFloorChanged.Broadcast(CurrentFloor);
    return true;
}

void UCardGameInstance::SetCurrentFloor(int32 NewFloor)
{
    if (CurrentFloor != NewFloor)
    {
        CurrentFloor = NewFloor;
        OnFloorChanged.Broadcast(CurrentFloor); // 背景、地图等所有绑定者自动更新
    }
}