#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RouteMapWidget.h"
#include "RouteMapBackground.h"
#include "CardGameInstance.generated.h"

class URouteMapManager;
class UMySaveGame;
class ULevelNode;

/** 楼层变化多播委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFloorChanged, int32, NewFloor);

UCLASS()
class AETHER_CARD_API UCardGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;
    virtual void Shutdown() override;

    // ---- 游戏流程 ----
    /** 开始新游戏：生成三层路线图，激活第一层起点，显示地图与背景 */
    UFUNCTION(BlueprintCallable, Category = "Game|Route")
    void Starting();

    // ---- 存档/读档 ----
    /** 保存游戏到指定存档槽 */
    UFUNCTION(BlueprintCallable, Category = "Game|Save")
    bool SaveGame(const FString& SlotName, int32 UserIndex = 0);

    /** 从存档槽加载游戏，恢复楼层并仅显示当前楼层地图 */
    UFUNCTION(BlueprintCallable, Category = "Game|Save")
    bool LoadGame(const FString& SlotName, int32 UserIndex = 0);

    // ---- 楼层查询与设置 ----
    /** 获取当前楼层（1 起始） */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game|Floor")
    int32 GetCurrentFloor() const { return CurrentFloor; }

    /** 设置当前楼层，触发 OnFloorChanged 委托 */
    UFUNCTION(BlueprintCallable, Category = "Game|Floor")
    void SetCurrentFloor(int32 NewFloor);

    // ---- 激活的关卡节点 ----
    UFUNCTION(BlueprintCallable, Category = "Game|Route")
    ULevelNode* GetCurrentActiveNode() const { return CurrentActiveNode; }

    // ---- 委托 ----
    /** 楼层变化时广播，已绑定地图和背景刷新 */
    UPROPERTY(BlueprintAssignable, Category = "Game|Floor")
    FOnFloorChanged OnFloorChanged;

    // ---- UI 蓝图类引用（在派生蓝图或默认值中设置） ----
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<URouteMapWidget> RouteMapWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<URouteMapBackground> RouteMapBackgroundClass;

protected:
    // ---- 核心对象 ----
    UPROPERTY()
    URouteMapManager* RouteMapManager;

    UPROPERTY()
    URouteMapWidget* RouteMapWidget;

    UPROPERTY()
    URouteMapBackground* RouteMapBackground;

    UPROPERTY()
    ULevelNode* CurrentActiveNode;

    UPROPERTY()
    int32 CurrentFloor = 1;      // 当前所在楼层，1-3
};