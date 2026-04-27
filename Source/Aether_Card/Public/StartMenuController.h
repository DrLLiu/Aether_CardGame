// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StartMenuController.generated.h"

/**
 * 
 */
UCLASS()
class AETHER_CARD_API AStartMenuController : public APlayerController
{
	GENERATED_BODY()
	

public:
	int32 CurrentRole;//0代表薇儿，1代表诗寇蒂，2代表千羽，3代表前鬼坊


public:
	UUserWidget* StartMenu;
	UUserWidget* ChooseRole;


public:
	virtual void BeginPlay()override;

	UFUNCTION(BlueprintCallable, Category = "Controler")
	void Startting();

public:
	int32 GetCurrentRole();
	void SetCurrentRole(int32 RoleNum);


};
