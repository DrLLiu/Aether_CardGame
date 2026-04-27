#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Button.h"
#include "RouteMapManager.h"
#include "LevelNode.h"
#include "RouteMapWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRouteNodeClicked, ULevelNode*, ClickedNode);

UCLASS()
class AETHER_CARD_API URouteMapWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "RouteMap")
    FOnRouteNodeClicked OnNodeClicked;

    UFUNCTION(BlueprintCallable, Category = "RouteMap")
    void BuildMapFromLayers(const TArray<FRouteMapLayer>& Layers, int32 InNumColumnsPerLayer);

    // 【新增】设置当前可见的最大楼层，并自动重新构建地图
    UFUNCTION(BlueprintCallable, Category = "RouteMap")
    void SetMaxVisibleFloor(int32 MaxFloor);

    UFUNCTION(BlueprintCallable, Category = "RouteMap")
    void ShowMap();

    UFUNCTION(BlueprintCallable, Category = "RouteMap")
    void HideMap();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
        int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* MapCanvas;

    // --- 编辑器可配置参数 ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RouteMap|Icons", meta = (AllowPrivateAccess = "true"))
    TMap<ELevelNodeType, UTexture2D*> NodeIcons;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RouteMap|Layout", meta = (AllowPrivateAccess = "true"))
    float HSpacing = 540.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RouteMap|Layout", meta = (AllowPrivateAccess = "true"))
    float VSpacing = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RouteMap|Layout", meta = (AllowPrivateAccess = "true"))
    float RowSpacing = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RouteMap|Layout", meta = (AllowPrivateAccess = "true"))
    float LeftMargin = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RouteMap|Layout", meta = (AllowPrivateAccess = "true"))
    float TopMargin = 270.0f;

    // 右边界留白部分
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RouteMap|Layout", meta = (AllowPrivateAccess = "true"))
    float HorizontalEdgePadding = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RouteMap|Layout", meta = (AllowPrivateAccess = "true"))
    FVector2D NodeButtonSize = FVector2D(300.0f, 130.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RouteMap|Layout", meta = (AllowPrivateAccess = "true"))
    float LineThickness = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RouteMap|Layout", meta = (AllowPrivateAccess = "true"))
    FLinearColor LineColor = FLinearColor(0.0f, 0.3f, 1.0f, 1.0f);

    // --- 运行时数据 ---
    UPROPERTY()
    TArray<UButton*> NodeButtons;

    TMap<ULevelNode*, FVector2D> NodeOriginalPositions;

    struct FConnectionLine
    {
        ULevelNode* From;
        ULevelNode* To;
    };
    TArray<FConnectionLine> Connections;

    FVector2D CurrentOffset;
    bool bIsDragging = false;
    FVector2D DragStartPosition;
    FVector2D DragStartOffset;

    // 边界改为水平方向
    float MinOffsetX = 0.0f;
    float MaxOffsetX = 0.0f;

    UPROPERTY()
    TMap<UButton*, ULevelNode*> ButtonToNode;

    UPROPERTY()
    UButton* LastClickedButton = nullptr;

    // 【新增】楼层控制
    UPROPERTY()
    TArray<FRouteMapLayer> CachedLayers;
    int32 CachedNumColumns = 0;
    int32 CurrentMaxVisibleFloor = 1;   // 初始只显示第一层

    void ClearMap();
    void UpdateNodePositions();
    void RecalculateBounds(const FGeometry& MyGeometry);
    bool IsClickOnButton(const FVector2D& LocalPos) const;


    //函数类型
    UFUNCTION()
    void OnButtonClicked();
public:
    UFUNCTION()
    void OnFloorChangedHandler(int32 NewFloor);
};