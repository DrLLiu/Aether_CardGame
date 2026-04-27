// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameMode.h"
#include "Blueprint/UserWidget.h"


void AMainGameMode::BeginPlay()
{
    Super::BeginPlay();

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC)
        {
            PC->bShowMouseCursor = true;
            FInputModeGameAndUI Mode;
            PC->SetInputMode(Mode);
        }
    }

    //调用GameInstance中的中的Starting函数
    UCardGameInstance* GI = Cast<UCardGameInstance>(GetGameInstance());
    if (GI)
    {
        GI->Starting();
    }
}
