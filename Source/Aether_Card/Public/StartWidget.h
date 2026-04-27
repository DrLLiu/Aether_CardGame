// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartWidget.generated.h"

/**
 * 
 */
UCLASS()
class AETHER_CARD_API UStartWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "StartWidget")
	float CurrentHealth=100.0f;//测试变量，不是真的要使用的
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StartWidget")
	float MaxHealth;

	UFUNCTION(BlueprintCallable, Category = "StartWidget")//仅在蓝图中可被调用
	void StartGame();//游戏开始，调用下一个UI

};
