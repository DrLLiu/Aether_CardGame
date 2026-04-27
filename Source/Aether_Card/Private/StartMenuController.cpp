// Fill out your copyright notice in the Description page of Project Settings.


#include "StartMenuController.h"
#include "Blueprint/UserWidget.h"



void AStartMenuController::BeginPlay() {
	Super::BeginPlay();
	CurrentRole = 0;//初始角色设定为0编号

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	//InputMode.SetHideCursorDuringCapture(false);

	SetInputMode(InputMode);
	bShowMouseCursor = true;


	UClass* StartMenuClass = LoadClass<UUserWidget>(this, TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/Start_Menu_UI.Start_Menu_UI_C'"));//获取开始菜单的UI资源

	StartMenu = CreateWidget<UUserWidget>(GetWorld(), StartMenuClass);//实例化

	StartMenu->AddToViewport();//将初始菜单渲染到屏幕上
}

void AStartMenuController::Startting()
{
	UClass* ChooseClass = LoadClass<UUserWidget>(this, TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/UI/ChooseRole.ChooseRole_C'"));//获取选择界面UI
	StartMenu->RemoveFromViewport();//移除初始菜单

	ChooseRole = CreateWidget<UUserWidget>(GetWorld(), ChooseClass);//实例化

    ChooseRole->AddToViewport();//渲染到屏幕上
}

int32 AStartMenuController::GetCurrentRole()
{
	int32 Rolenum = CurrentRole;
	return Rolenum;
}

void AStartMenuController::SetCurrentRole(int32 RoleNum)
{
	CurrentRole= RoleNum;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("CurrentRole is %d"), RoleNum));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("CurrentRole is %d"), RoleNum));
}
