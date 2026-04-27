// Fill out your copyright notice in the Description page of Project Settings.


#include "ChooseRoleWidget.h"
#include "CardGameInstance.h"
#include "StartMenuController.h"
#include "Kismet/GameplayStatics.h"

void UChooseRoleWidget::CurrentRoleNumber(int32 RoleNumber)
{
    APlayerController* PC = GetOwningPlayer();//获取当前场景的玩家控制器

    // 转换并调用
    if (AStartMenuController* MyPC = Cast<AStartMenuController>(PC))//转换类型为自定义的PlayerController类，成功的话执行下一步
    {
        MyPC->SetCurrentRole(RoleNumber);//将当前角色设置为
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController is not a StartMenuController!"));
    }
}

void UChooseRoleWidget::ChangeMap()
{
    UGameplayStatics::OpenLevel(this, TEXT("Fight_Map"));


    //这个功能暂时废弃掉，调用Starting函数的部分给GameMode了，后续这里还是要和GameMode联动，暂时先废弃后续启用
    //获取GameInstance并且调用新游戏开始
    //UGameInstance* GI = GetGameInstance();
    //UCardGameInstance* CardGI = Cast<UCardGameInstance>(GetGameInstance());
    //if (CardGI)
    //{
    //    CardGI->Starting();  // 调用自定义函数
    //    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("send")));
    //}
}


