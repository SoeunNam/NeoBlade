//Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DroneEnemy.generated.h"

UCLASS()
class NEOBLADE_API ADroneEnemy : public ACharacter
{
	GENERATED_BODY()

UPROPERTY(EditAnywhere)
	class USphereComponent* sphereComp;
protected:
	
	ADroneEnemy();

	virtual void BeginPlay() override;

public:
	
	virtual void Tick(float DeltaTime) override;

	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	
	UPROPERTY(VisibleAnywhere, Category = FSM)
	class APlayerCharacter* target;

	
	UPROPERTY()
	class ADroneEnemy* me;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FSMComponent)
	class UDroneEnemyFSM* fsm;

};
