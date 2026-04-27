// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CardGameInstance.h"
#include "GameFramework/GameMode.h"
#include "MainGameMode.generated.h"

/**
 * 
 */
UCLASS()
class AETHER_CARD_API AMainGameMode : public AGameMode
{
	GENERATED_BODY()
	

	virtual void BeginPlay() override;
};
