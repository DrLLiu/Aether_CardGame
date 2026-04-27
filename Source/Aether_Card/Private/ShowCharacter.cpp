// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "StartMenuController.h" 

// Sets default values
AShowCharacter::AShowCharacter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 创建四个骨骼网格体组件
	SkeletalMesh0 = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh0"));
	SkeletalMesh1 = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh1"));
	SkeletalMesh2 = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh2"));
	SkeletalMesh3 = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh3"));

	// 设置根组件，并将四个网格体附加到根组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SkeletalMesh0->SetupAttachment(RootComponent);
	SkeletalMesh1->SetupAttachment(RootComponent);
	SkeletalMesh2->SetupAttachment(RootComponent);
	SkeletalMesh3->SetupAttachment(RootComponent);

	// 初始化当前索引为-1，表示未设置
	CurrentMeshIndex = -1;
	StartMenuController = nullptr;

}

// Called when the game starts or when spawned
void AShowCharacter::BeginPlay()
{
	Super::BeginPlay();
	

	// 获取StartMenuController
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		StartMenuController = Cast<AStartMenuController>(PC);
	}

	// 初始隐藏所有网格体
	SkeletalMesh0->SetVisibility(false, true);
	SkeletalMesh1->SetVisibility(false, true);
	SkeletalMesh2->SetVisibility(false, true);
	SkeletalMesh3->SetVisibility(false, true);
}

// Called every frame
void AShowCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (!StartMenuController)
	{
		return;
	}

	// 获取当前角色索引
	int32 RoleIndex = StartMenuController->GetCurrentRole();

	// 检查索引是否在有效范围内
	if (RoleIndex < 0 || RoleIndex > 3)
	{
		return;
	}

	// 如果索引发生了变化，则更新可见性
	if (CurrentMeshIndex != RoleIndex)
	{
		SetSkeletalMeshVisibility(RoleIndex);
		CurrentMeshIndex = RoleIndex;
	}
}

void AShowCharacter::SetSkeletalMeshVisibility(int32 Index)//显示所选中的网格体
{
	// 首先隐藏所有网格体
	SkeletalMesh0->SetVisibility(false, true);
	SkeletalMesh1->SetVisibility(false, true);
	SkeletalMesh2->SetVisibility(false, true);
	SkeletalMesh3->SetVisibility(false, true);

	// 根据索引显示对应的网格体
	switch (Index)
	{
	case 0:
		SkeletalMesh0->SetVisibility(true, true);
		break;
	case 1:
		SkeletalMesh1->SetVisibility(true, true);
		break;
	case 2:
		SkeletalMesh2->SetVisibility(true, true);
		break;
	case 3:
		SkeletalMesh3->SetVisibility(true, true);
		break;
	default:
		break;
	}
}

