// Fill out your copyright notice in the Description page of Project Settings.


#include "StopAttack.h"
#include "Boss1.h"

void UStopAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if(ABoss1* Boss = Cast<ABoss1>(MeshComp->GetOwner()))
	{
		// PhaseChange 중이면 Idle로 바꾸지 마라!!!!
		if (Boss->BossState == EBossState::PhaseChange)
			return;

		Boss->StopAttack();
	}
}
