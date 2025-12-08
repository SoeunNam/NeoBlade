// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeleeEnemy.h"
#include "SwordTrailActor.generated.h"

UCLASS()
class NEOBLADE_API ASwordTrailActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASwordTrailActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// 검 Mesh
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* SwordMesh;

	// 트레일 Niagara
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* TrailEffect;

	// Niagara Parameter 이름
	FName TipParam = TEXT("User.SwordTip");
	FName BaseParam = TEXT("User.SwordBase");

	// Trail 제어 함수
	void EnableTrail();
	void DisableTrail();

	UFUNCTION()
	void SwordLineTrace();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 10.f;

	UPROPERTY()
	AMeleeEnemy* OwnerCharacter = nullptr;
};
