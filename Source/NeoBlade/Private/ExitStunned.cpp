// Fill out your copyright notice in the Description page of Project Settings.


#include "ExitStunned.h"
#include "Boss1.h"

void UExitStunned::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (ABoss1* Boss = Cast<ABoss1>(MeshComp->GetOwner()))
	{
		Boss->ExitStunned();
	}
}
