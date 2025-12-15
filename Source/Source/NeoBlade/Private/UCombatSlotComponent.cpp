// Fill out your copyright notice in the Description page of Project Settings.


#include "UCombatSlotComponent.h"
// 소유 액터 / 폰 관련
#include "GameFramework/Pawn.h"

// AI 관련
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

// 매니저 컴포넌트
#include "UCombatManagerComponent.h"
#include"MeleeEnemy.h"

// Sets default values for this component's properties
UUCombatSlotComponent::UUCombatSlotComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUCombatSlotComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UUCombatSlotComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UUCombatSlotComponent::RequestAttack()
{
	if (Manager)
	{
		Manager->RequestAttack(this);
	}
}

void UUCombatSlotComponent::SetAttackPermission(bool bValue)
{
	bCanAttack = bValue;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* Pawn = Cast<APawn>(Owner);
	if (!Pawn) return;
	AMeleeEnemy* enemy = Cast<AMeleeEnemy>(Pawn);
	if (AAIController* AI = Cast<AAIController>(Pawn->GetController()))
	{
		if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
		{
			/*BB->SetValueAsBool("bIsAttack", bCanAttack);*/
			/*enemy->UpdateDetectState(bCanAttack);*/
		}

	}
}

