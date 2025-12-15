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
    if (ABoss1* Boss = Cast<ABoss1>(GetOwner()))
    {
        // PhaseChange 중이면 무시
        if (Boss->BossState == EBossState::PhaseChange)
        {
            UE_LOG(LogTemp, Warning, TEXT("[HitReact] Blocked — PhaseChange ongoing"));
            return;
        }

        // 이미 Hit이면 중복 방지
        if (Boss->BossState == EBossState::Hit)
            return;

        Boss->SetBossState(EBossState::Hit);
        Boss->HitReactPlay();

        // 안전: 타이머 종료 시 PhaseChange면 Idle로 바꾸지 않음
        GetWorld()->GetTimerManager().SetTimer(
            HitTimer,
            FTimerDelegate::CreateLambda([Boss]()
                {
                    if (!IsValid(Boss)) return;
                    if (Boss->BossState == EBossState::PhaseChange) return;
                    Boss->SetBossState(EBossState::Idle);
                }),
            0.5f,
            false
        );
    }
}

