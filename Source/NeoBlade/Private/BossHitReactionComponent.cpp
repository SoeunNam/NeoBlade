// Fill out your copyright notice in the Description page of Project Settings.


#include "BossHitReactionComponent.h"
#include "Boss1.h"
#include "BossHealthComponent.h"

// Sets default values for this component's properties
UBossHitReactionComponent::UBossHitReactionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBossHitReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	// Mesh 가져오기
	ABoss1* OwnerChar = Cast<ABoss1>(GetOwner());
	if (OwnerChar)
	{
		MeshComp = OwnerChar->GetMesh();
	}

	if (!MeshComp)
	{
		UE_LOG(LogTemp, Error, TEXT("Boss Mesh 없음!"));
	}
}


// Called every frame
void UBossHitReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UBossHitReactionComponent::PlayHitReact()
{
	// 보스 주인 가져오기
	if (ABoss1* Boss = Cast<ABoss1>(GetOwner()))
	{
		//UE_LOG(LogTemp, Warning, TEXT("PlayHitReact() called"));
		// 보스 상태를 Hit으로 변경
		Boss->SetBossState(EBossState::Hit);
		Boss->HitReactPlay();
		// Hit 상태 최소 0.5초 유지
		Boss->GetWorldTimerManager().SetTimer(
			HitTimer,
			FTimerDelegate::CreateLambda([Boss]()
				{
					Boss->SetBossState(EBossState::Idle);
				}),
			0.5f,
			false
		);


		// Optional: Hit 애니 재생이 ABP에서 State Machine으로 연결되어 있다면, 몽타주 따로 안해도 됨
	}
}

