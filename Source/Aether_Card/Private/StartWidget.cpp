// Fill out your copyright notice in the Description page of Project Settings.


#include "StartWidget.h"
#include "StartMenuController.h"

void UStartWidget::StartGame()
{
    APlayerController* PC = GetOwningPlayer();//获取当前场景的玩家控制器

    // 转换并调用
    if (AStartMenuController* MyPC = Cast<AStartMenuController>(PC))//转换类型为自定义的PlayerController类，成功的话执行下一步
    {
        MyPC->Startting();
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController is not a StartMenuController!"));
    }
}
