// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetMarkerComponent.h"

// Sets default values for this component's properties
UTargetMarkerComponent::UTargetMarkerComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}


// Called when the game starts
void UTargetMarkerComponent::BeginPlay()
{
    Super::BeginPlay();

    // ...

    AActor* Owner = GetOwner();
    if (!Owner) return;

    MarkerMesh = NewObject<UStaticMeshComponent>(Owner, TEXT("TargetMarkerMesh"));
    MarkerMesh->SetupAttachment(Owner->GetRootComponent());
    MarkerMesh->RegisterComponent();

    // 삼각형 메쉬 세팅 (직접 만든 Mesh 또는 Engine 기본 Cone 사용 가능)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(
        TEXT("/Engine/BasicShapes/Cone.Cone")   // ↓ 원하는 삼각형 mesh 가능
    );

    if (MeshAsset.Succeeded())
        MarkerMesh->SetStaticMesh(MeshAsset.Object);

    // 위치 + 스케일
    MarkerMesh->SetRelativeLocation(FVector(0, 0, 180));
    MarkerMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));

    // 충돌/그림자 제거
    MarkerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MarkerMesh->SetCastShadow(false);


    // 기본 숨김
    MarkerMesh->SetVisibility(false);



}


// Called every frame
void UTargetMarkerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}


void UTargetMarkerComponent::ShowMarker()
{
    if (MarkerMesh)
        MarkerMesh->SetVisibility(true);
}

void UTargetMarkerComponent::HideMarker()
{
    if (MarkerMesh)
        MarkerMesh->SetVisibility(false);
}