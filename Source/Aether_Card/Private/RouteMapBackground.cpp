#include "RouteMapBackground.h"
#include "RouteMapWidget.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void URouteMapBackground::NativeConstruct()
{
    Super::NativeConstruct();

    SetVisibility(ESlateVisibility::Visible);

    if (!Back)
    {
        UE_LOG(LogTemp, Warning, TEXT("RouteMapBackground: 'Back' Image not bound!"));
    }
    UE_LOG(LogTemp, Warning, TEXT("Background created, Back valid: %s"), Back ? TEXT("Yes") : TEXT("No"));
}

void URouteMapBackground::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 同步可见性：当关联的 RouteMapWidget 改变可见性时，自动跟随
    if (SyncedRouteMapWidget.IsValid())
    {
        ESlateVisibility TargetVis = SyncedRouteMapWidget->GetVisibility();
        if (GetVisibility() != TargetVis)
        {
            SetVisibility(TargetVis);
        }
    }
}

void URouteMapBackground::SetFloor(int32 Floor)
{
    int32 Index = FMath::Clamp(Floor - 1, 0, FloorBackgrounds.Num() - 1);
    if (FloorBackgrounds.IsValidIndex(Index) && Back)
    {
        UTexture2D* Tex = FloorBackgrounds[Index];
        if (Tex)
        {
            Back->SetBrushFromTexture(Tex);
        }
    }
}

void URouteMapBackground::SyncWithRouteMapWidget(URouteMapWidget* InRouteMapWidget)
{
    SyncedRouteMapWidget = InRouteMapWidget;
    // 初始同步可见性
    if (SyncedRouteMapWidget.IsValid())
    {
        SetVisibility(SyncedRouteMapWidget->GetVisibility());
    }
}

//由GameInstance中的代理来绑定
void URouteMapBackground::OnFloorChangedHandler(int32 NewFloor)
{
    SetFloor(NewFloor);
}