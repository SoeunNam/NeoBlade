// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnim.h"
#include "MeleeEnemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SwordTrailActor.h"

void UEnemyAnim::AnimNotify_SlashStart()
{
	AMeleeEnemy* Owner = Cast<AMeleeEnemy>(TryGetPawnOwner());
	if (Owner)
	{
		Owner->PlaySlashEffect(1);
		/*Owner->LineTrace();*/
	}
}

void UEnemyAnim::AnimNotify_SlashStart2()
{
	AMeleeEnemy* Owner = Cast<AMeleeEnemy>(TryGetPawnOwner());
	if (Owner)
	{
		Owner->PlaySlashEffect(2);
		/*Owner->LineTrace();*/
	}
}

void UEnemyAnim::AnimNotify_End_Slash()
{
	AMeleeEnemy* Owner = Cast<AMeleeEnemy>(TryGetPawnOwner());
	if (Owner)
	{


		Owner->OnAttackEnd();
		Owner->bIsAttacking = false;

		ASwordTrailActor* Sword = Cast<ASwordTrailActor>(Owner->SwordActor);
		if (!Sword) return;

		Sword->StopTrail();
	}



}


void UEnemyAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn) return;

	AMeleeEnemy* Enemy = Cast<AMeleeEnemy>(Pawn);
	if (!Enemy) return;

	Speed = Pawn->GetVelocity().Size();
	bHasTarget = Enemy->bHasTarget;
	bIsDead = Enemy->bIsDead;
	Circle = Enemy->bIsReady;
	Direction = Enemy->CurrentOrbitDirection;
}

