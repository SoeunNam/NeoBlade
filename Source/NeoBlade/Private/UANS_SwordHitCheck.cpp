// Fill out your copyright notice in the Description page of Project Settings.


#include "UANS_SwordHitCheck.h"
#include"SwordTrailActor.h"
#include"MeleeEnemy.h"
#include "GameFramework/Actor.h"

void UUANS_SwordHitCheck::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	UE_LOG(LogTemp, Warning, TEXT("Sword Notify Begin"));
}

void UUANS_SwordHitCheck::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float DeltaTime)
{
	// 매 프레임 실행
	if (!MeshComp) return;

	// 소유 액터(= 적)
	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;
	AMeleeEnemy* meleeEnemy = Cast<AMeleeEnemy>(Owner);
	if (!meleeEnemy) return;
	// 적이 들고 있는 검 가져오기
	ASwordTrailActor* Sword = Cast<ASwordTrailActor>(meleeEnemy->SwordActor);
	if (!Sword) return;

	// Sword 클래스 내부의 충돌/데미지 체크 함수 실행
	Sword->SwordLineTrace();   // ← 네가 만든 함수 이름 넣기
}

void UUANS_SwordHitCheck::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	UE_LOG(LogTemp, Warning, TEXT("Sword Notify End"));
}