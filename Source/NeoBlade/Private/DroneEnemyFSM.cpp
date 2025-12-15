// Fill out your copyright notice in the Description page of Project Settings.


#include "DroneEnemyFSM.h"

#include "PlayerCharacter.h"
#include "DroneEnemy.h"

#include "Particles/ParticleSystem.h"
#include "NiagaraFunctionLibrary.h"

#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>

#include <DamageComponent.h>


// Sets default values for this component's properties
UDroneEnemyFSM::UDroneEnemyFSM()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


}


// Called when the game starts
void UDroneEnemyFSM::BeginPlay()
{
	Super::BeginPlay();

	auto actor = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerCharacter::StaticClass());
	target = Cast<APlayerCharacter>(actor);

	me = Cast<ADroneEnemy>(GetOwner());
	
}


// Called every frame
void UDroneEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	currentTime += DeltaTime;

	switch (mState)
	{
	case EEnemyState::Idle:
		IdleState();
		break;
	case EEnemyState::Move:
		MoveState(DeltaTime);
		break;
	case EEnemyState::Attack:
		AttackState();
		break;
	case EEnemyState::Damage:
		DamageState();
		break;
	case EEnemyState::Die:
		DieState();
		break;
	default:
		break;
	}
}

void UDroneEnemyFSM::IdleState()
{
	if (currentTime > idleDelayTime)
	{
		mState = EEnemyState::Move;
		currentTime = 0;

	}
}

void UDroneEnemyFSM::MoveState(float DeltaTime)
{

	FVector Direction = target->GetActorLocation() - me->GetActorLocation();

	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(me->GetActorLocation(), target->GetActorLocation());

	// Smoothly interpolate the current rotation towards the target rotation.
	// We only care about the Yaw (Z-axis rotation) for facing the target horizontally.
	FRotator CurrentRotation = me->GetActorRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f); // Adjust interp speed (5.0f) as needed

	// Apply the new rotation to the drone actor.
	me->SetActorRotation(NewRotation);

	if (Direction.Size() <= attackRange)
	{
		mState = EEnemyState::Attack;
	}
	else
	{
		if (target != nullptr && me != nullptr)
		{
			me->AddMovementInput(Direction.GetSafeNormal());
		}
	}
}

void UDroneEnemyFSM::AttackState()
{
	FVector Direction = target->GetActorLocation() - me->GetActorLocation();

	if (Direction.Size() <= attackRange)
	{
		if (currentTime > attackDelayTime)
		{
			UE_LOG(LogTemp, Warning, TEXT("Attack State!!"));

			//Damage to Player

			AActor* HitActor = target;     // ← ?? 맞은 액터

			if (HitActor)
			{
				if (HitActor->ActorHasTag("Player"))
				{
					if (UDamageComponent* Dmg = HitActor->FindComponentByClass<UDamageComponent>())
					{
						Dmg->DealDamage(10, me);
					}
				}
			}


			if (NiagaraExplosionEffect != nullptr)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					GetWorld(),
					NiagaraExplosionEffect,
					me->GetActorLocation(),		//FVector Location
					me->GetActorRotation()		//FVector Rotation
				);
			}

			mState = EEnemyState::Die;
		}
	}
	else
	{
		mState = EEnemyState::Move;
		currentTime = 0;
	}

}

void UDroneEnemyFSM::DamageState()
{
	if (hp <= 0)
	{
		mState = EEnemyState::Die;
	}
}

void UDroneEnemyFSM::DieState()
{
	if (me != nullptr)
	{
		// Effect 적용
		//if (ExplosionEffect != nullptr)
		//{
		//	// UParticleSystem (Cascade) 스폰
		//	UGameplayStatics::SpawnEmitterAtLocation(
		//		GetWorld(),
		//		ExplosionEffect,
		//		me->GetActorLocation(),
		//		me->GetActorRotation()
		//	);
		//	
		//}

		// Sound
		//UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSoundWave, me->GetActorLocation());

		me->Destroy();
	}
}
