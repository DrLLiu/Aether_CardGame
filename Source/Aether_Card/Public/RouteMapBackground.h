#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "RouteMapBackground.generated.h"

class URouteMapWidget;

UCLASS()
class AETHER_CARD_API URouteMapBackground : public UUserWidget
{
    GENERATED_BODY()

public:
    // 背景图片资源（按楼层索引 0,1,2 对应 1,2,3 层）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Background")
    TArray<UTexture2D*> FloorBackgrounds;

    /** 设置当前楼层，切换背景图片（索引 0 为基础） */
    UFUNCTION(BlueprintCallable, Category = "Background")
    void SetFloor(int32 Floor);

    /** 与 RouteMapWidget 同步可见性（通过 Weak 引用实现） */
    UFUNCTION(BlueprintCallable, Category = "Background")
    void SyncWithRouteMapWidget(URouteMapWidget* InRouteMapWidget);

public:
    /** 响应楼层变化（绑定 OnFloorChanged 委托） */
    UFUNCTION()
    void OnFloorChangedHandler(int32 NewFloor);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    //子类中已创建该图片
    UPROPERTY(meta = (BindWidget))
    UImage* Back; 

    TWeakObjectPtr<URouteMapWidget> SyncedRouteMapWidget;
};