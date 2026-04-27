// Fill out your copyright notice in the Description page of Project Settings.


#include "BackPlane.h"
#include "Components/StaticMeshComponent.h"
#include "StartMenuController.h"  // 请确保包含你的StartMenuController头文件
#include "Engine/StaticMesh.h"

// Sets default values
ABackPlane::ABackPlane()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 创建默认的平面网格体组件
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
	RootComponent = PlaneMesh;

	// 设置默认的平面网格体（使用引擎自带的平面）
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMeshAsset.Succeeded())
	{
		PlaneMesh->SetStaticMesh(PlaneMeshAsset.Object);
	}



	// 初始化材质数组大小为4
	Materials.SetNum(4);

	CurrentMaterialIndex = -1; // 初始化为无效值
	StartMenuController = nullptr;
}

// Called when the game starts or when spawned
void ABackPlane::BeginPlay()
{
	Super::BeginPlay();


	// 获取当前关卡中的PlayerController并转换为StartMenuController
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		StartMenuController = Cast<AStartMenuController>(PC);
	}
	
}

// Called every frame
void ABackPlane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!StartMenuController)
	{
		return; // 如果没有获取到StartMenuController，直接返回
	}

	// 从StartMenuController获取当前角色索引
	int32 RoleIndex = StartMenuController->GetCurrentRole();

	// 确保索引在0-3范围内(后续可能进行扩展此处记得修改)
	if (RoleIndex < 0 || RoleIndex >= 4)
	{
		return; // 索引无效
	}

	// 如果索引发生变化，则更新材质
	if (CurrentMaterialIndex != RoleIndex && Materials.IsValidIndex(RoleIndex) && Materials[RoleIndex])
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("change to %d"), RoleIndex));
		PlaneMesh->SetMaterial(0, Materials[RoleIndex]);
		CurrentMaterialIndex = RoleIndex;
	}

}

