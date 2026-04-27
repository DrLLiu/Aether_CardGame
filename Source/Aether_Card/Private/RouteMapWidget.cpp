#include "RouteMapWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"

void URouteMapWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (!MapCanvas)
    {
        UE_LOG(LogTemp, Error, TEXT("RouteMapWidget: MapCanvas is null!"));
    }
}

void URouteMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    RecalculateBounds(MyGeometry);
}

int32 URouteMapWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
    int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    // 先绘制连线（按钮下层）
    if (!Connections.IsEmpty())
    {
        const FVector2D WidgetOffset = AllottedGeometry.AbsoluteToLocal(AllottedGeometry.GetAbsolutePosition());
        const FVector2D HalfSize = NodeButtonSize * 0.5f;   // 半宽、半高
        for (const FConnectionLine& Line : Connections)
        {
            const FVector2D* FromPos = NodeOriginalPositions.Find(Line.From);
            const FVector2D* ToPos = NodeOriginalPositions.Find(Line.To);
            if (!FromPos || !ToPos) continue;
            // 按钮中心坐标（世界位置）
            FVector2D CenterFrom = *FromPos + HalfSize + CurrentOffset + WidgetOffset;
            FVector2D CenterTo = *ToPos + HalfSize + CurrentOffset + WidgetOffset;
            // 从 From 指向 To 的方向向量
            FVector2D Dir = CenterTo - CenterFrom;
            if (Dir.IsNearlyZero()) continue;
            // ─── 精确计算射线与自身矩形的交点（边框出发） ───
            auto CalcEdgePoint = [&](const FVector2D& Center, const FVector2D& Direction) -> FVector2D
                {
                    // 求解 t 使得 Center + t * Direction 落在矩形边框上
                    // 矩形半尺寸为 HalfSize
                    float tMin = FLT_MAX;
                    // 检查与左右边框的交点
                    if (FMath::Abs(Direction.X) > KINDA_SMALL_NUMBER)
                    {
                        float t = HalfSize.X / FMath::Abs(Direction.X);   // 到达垂直边的参数
                        tMin = FMath::Min(tMin, t);
                    }
                    // 检查与上下边框的交点
                    if (FMath::Abs(Direction.Y) > KINDA_SMALL_NUMBER)
                    {
                        float t = HalfSize.Y / FMath::Abs(Direction.Y);   // 到达水平边的参数
                        tMin = FMath::Min(tMin, t);
                    }
                    if (tMin == FLT_MAX) return Center;
                    return Center + Direction * tMin;
                };
            FVector2D P0 = CalcEdgePoint(CenterFrom, Dir);       // From 按钮向着 To 的方向碰到的边框点
            FVector2D P1 = CalcEdgePoint(CenterTo, -Dir);      // To 按钮向着 From 的方向碰到的边框点
            TArray<FVector2D> Points;
            Points.Add(P0);
            Points.Add(P1);
            FSlateDrawElement::MakeLines(OutDrawElements, LayerId,
                AllottedGeometry.ToPaintGeometry(),
                Points, ESlateDrawEffect::None, LineColor, true, LineThickness);
        }
    }
    // 再绘制子控件（按钮）保证按钮覆盖在线上
    LayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements,
        LayerId + 1, InWidgetStyle, bParentEnabled);
    return LayerId;
}

FReply URouteMapWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        const FVector2D LocalPos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

        // 查找被点击的按钮
        LastClickedButton = nullptr;
        for (UButton* Btn : NodeButtons)
        {
            if (!Btn) continue;
            if (UCanvasPanelSlot* ButtonSlot = Cast<UCanvasPanelSlot>(Btn->Slot))
            {
                const FVector2D BtnPos = ButtonSlot->GetPosition();
                const FVector2D BtnSize = ButtonSlot->GetSize();
                if (LocalPos.X >= BtnPos.X && LocalPos.X <= BtnPos.X + BtnSize.X &&
                    LocalPos.Y >= BtnPos.Y && LocalPos.Y <= BtnPos.Y + BtnSize.Y)
                {
                    LastClickedButton = Btn;
                    break;
                }
            }
        }

        if (LastClickedButton)
        {
            return FReply::Unhandled();
        }
        else
        {
            bIsDragging = true;
            DragStartPosition = LocalPos;
            DragStartOffset = CurrentOffset;
            return FReply::Handled().CaptureMouse(TakeWidget());
        }
    }
    return FReply::Unhandled();
}

FReply URouteMapWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bIsDragging && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        bIsDragging = false;
        return FReply::Handled().ReleaseMouseCapture();
    }
    return FReply::Unhandled();
}

FReply URouteMapWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (bIsDragging)
    {
        const FVector2D LocalPos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
        const FVector2D Delta = LocalPos - DragStartPosition;
        CurrentOffset = DragStartOffset + Delta;

        // 只允许水平拖动
        CurrentOffset.X = FMath::Clamp(CurrentOffset.X, MinOffsetX, MaxOffsetX);
        CurrentOffset.Y = 0.0f;

        UpdateNodePositions();
        Invalidate(EInvalidateWidget::Paint);
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

void URouteMapWidget::BuildMapFromLayers(const TArray<FRouteMapLayer>& Layers, int32 InNumColumnsPerLayer)
{
    if (!MapCanvas) return;
    CachedLayers = Layers;
    CachedNumColumns = InNumColumnsPerLayer;
    ClearMap();
    const int32 MaxFloor = CurrentMaxVisibleFloor; // 单层显示，参考楼层就是当前层
    for (const FRouteMapLayer& Layer : Layers)
    {
        for (ULevelNode* Node : Layer.Nodes)
        {
            // 单层过滤：只选择当前可见楼层
            if (!Node || Node->FloorLevel != CurrentMaxVisibleFloor) continue;
            const FIntPoint* Pos = Layer.Positions.Find(Node);
            if (!Pos) continue;
            const int32 Column = Pos->X;
            const int32 Row = Pos->Y;
            // Y坐标不再依赖楼层差，单纯基于TopMargin和行间距
            const float Y = TopMargin + Row * RowSpacing;
            const float X = LeftMargin + Column * HSpacing;
            NodeOriginalPositions.Add(Node, FVector2D(X, Y));
            // 连线：只保留目标也在当前层的连接
            for (ULevelNode* Child : Node->ChildNodes)
            {
                if (Child && Child->FloorLevel == CurrentMaxVisibleFloor)
                {
                    Connections.Add({ Node, Child });
                }
            }
        }
    }
    // 创建按钮（后续代码与之前完全相同）
    for (const auto& Pair : NodeOriginalPositions)
    {
        ULevelNode* Node = Pair.Key;
        const FVector2D& Pos = Pair.Value;
        UButton* Button = NewObject<UButton>(this);
        if (!Button) continue;
        ButtonToNode.Add(Button, Node);
        FButtonStyle ButtonStyle = Button->WidgetStyle;
        UTexture2D** TexPtr = NodeIcons.Find(Node->NodeType);
        if (TexPtr && *TexPtr)
        {
            FSlateBrush Brush;
            Brush.SetResourceObject(*TexPtr);
            Brush.ImageSize = NodeButtonSize;
            ButtonStyle.Normal = Brush;
            ButtonStyle.Hovered = Brush;
            ButtonStyle.Pressed = Brush;
            Button->SetStyle(ButtonStyle);
        }
        Button->OnClicked.AddDynamic(this, &URouteMapWidget::OnButtonClicked);
        MapCanvas->AddChildToCanvas(Button);
        if (UCanvasPanelSlot* ButtonSlot = Cast<UCanvasPanelSlot>(Button->Slot))
        {
            ButtonSlot->SetZOrder(100);          // 可选，保持按钮上层
            ButtonSlot->SetPosition(Pos + CurrentOffset);
            ButtonSlot->SetSize(NodeButtonSize);
        }
        NodeButtons.Add(Button);
    }
    CurrentOffset = FVector2D::ZeroVector;
    UpdateNodePositions();
    RecalculateBounds(GetCachedGeometry());
}

void URouteMapWidget::SetMaxVisibleFloor(int32 MaxFloor)
{
    if (MaxFloor == CurrentMaxVisibleFloor) return;
    CurrentMaxVisibleFloor = FMath::Max(1, MaxFloor);
    // 重新构建地图（使用缓存数据）
    if (CachedLayers.Num() > 0)
    {
        BuildMapFromLayers(CachedLayers, CachedNumColumns);
    }
}

void URouteMapWidget::ShowMap()
{
    SetVisibility(ESlateVisibility::Visible);
}

void URouteMapWidget::HideMap()
{
    SetVisibility(ESlateVisibility::Collapsed);
}

void URouteMapWidget::ClearMap()
{
    if (MapCanvas)
    {
        MapCanvas->ClearChildren();
    }
    NodeButtons.Empty();
    NodeOriginalPositions.Empty();
    Connections.Empty();
    ButtonToNode.Empty();
    LastClickedButton = nullptr;
    CurrentOffset = FVector2D::ZeroVector;
    bIsDragging = false;
}

void URouteMapWidget::UpdateNodePositions()
{
    int32 Index = 0;
    for (const auto& Pair : NodeOriginalPositions)
    {
        if (Index < NodeButtons.Num())
        {
            UButton* Button = NodeButtons[Index];
            if (Button && Button->Slot)
            {
                if (UCanvasPanelSlot* ButtonSlot = Cast<UCanvasPanelSlot>(Button->Slot))
                {
                    ButtonSlot->SetPosition(Pair.Value + CurrentOffset);
                }
            }
        }
        Index++;
    }
}

void URouteMapWidget::RecalculateBounds(const FGeometry& MyGeometry)
{
    if (NodeOriginalPositions.IsEmpty())
    {
        MinOffsetX = 0.0f;
        MaxOffsetX = 0.0f;
        return;
    }

    float MinContentX = FLT_MAX;
    float MaxContentX = -FLT_MAX;

    for (const auto& Pair : NodeOriginalPositions)
    {
        const float HalfW = NodeButtonSize.X * 0.5f;
        const float LeftX = Pair.Value.X - HalfW;
        const float RightX = Pair.Value.X + HalfW;
        MinContentX = FMath::Min(MinContentX, LeftX);
        MaxContentX = FMath::Max(MaxContentX, RightX);
    }

    //边缘计算公式
    const float ViewportWidth = MyGeometry.GetLocalSize().X;
    MinOffsetX = ViewportWidth - MaxContentX - HorizontalEdgePadding;  // 允许向左多拖一点
    MaxOffsetX = -MinContentX + HorizontalEdgePadding;
}

bool URouteMapWidget::IsClickOnButton(const FVector2D& LocalPos) const
{
    for (UButton* Button : NodeButtons)
    {
        if (!Button) continue;
        if (UCanvasPanelSlot* ButtonSlot = Cast<UCanvasPanelSlot>(Button->Slot))
        {
            const FVector2D Pos = ButtonSlot->GetPosition();
            const FVector2D Size = ButtonSlot->GetSize();
            if (LocalPos.X >= Pos.X && LocalPos.X <= Pos.X + Size.X &&
                LocalPos.Y >= Pos.Y && LocalPos.Y <= Pos.Y + Size.Y)
            {
                return true;
            }
        }
    }
    return false;
}

void URouteMapWidget::OnFloorChangedHandler(int32 NewFloor)
{
    SetMaxVisibleFloor(NewFloor);
}

void URouteMapWidget::OnButtonClicked()
{
    if (!LastClickedButton) return;
    ULevelNode** NodePtr = ButtonToNode.Find(LastClickedButton);
    if (NodePtr && *NodePtr)
    {
        OnNodeClicked.Broadcast(*NodePtr);
    }
}