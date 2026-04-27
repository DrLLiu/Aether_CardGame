// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChooseRoleWidget.generated.h"

/**
 * 
 */
UCLASS()
class AETHER_CARD_API UChooseRoleWidget : public UUserWidget
{
	GENERATED_BODY()
	
	//int32 RoleNumber;
    //int32 RoleNumber2;

public:
	UFUNCTION(BlueprintCallable, Category = "Choose")
	void CurrentRoleNumber(int32 RoleNumber);

	UFUNCTION(BlueprintCallable, Category = "Choose")
	void ChangeMap();

};
