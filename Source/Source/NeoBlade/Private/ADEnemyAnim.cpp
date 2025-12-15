// Fill out your copyright notice in the Description page of Project Settings.


#include "ADEnemyAnim.h"
#include "ADEnemy.h"
#include "Kismet/GameplayStatics.h"     // GetPlayerPawn(), GetPlayerController(), etc
#include "GameFramework/Pawn.h"         // APawn
#include "GameFramework/Character.h"    // ACharacter (캐스팅할 때 필요)
#include "GameFramework/Actor.h"        // GetActorLocation()
#include "Kismet/KismetMathLibrary.h"   // MakeRotFromX()
#include"PlayerCharacter.h"

void UADEnemyAnim::AnimNotify_End_Attack()
{
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn) return;

	AADEnemy* Enemy = Cast<AADEnemy>(Pawn);
	if (!Enemy) return;
	Enemy->UpdateAttackState(false);
}

void UADEnemyAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn) return;

	AADEnemy* Enemy = Cast<AADEnemy>(Pawn);
	if (!Enemy) return;
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	APlayerCharacter* MyPlayer = Cast<APlayerCharacter>(PlayerPawn);
	if (PlayerPawn)
	{
		if (MyPlayer)
		{
			FVector Direction = MyPlayer->GetActorLocation() - Enemy->GetActorLocation();

			FRotator LookRot = UKismetMathLibrary::MakeRotFromX(Direction);

			// AimOffset 변수에 넣기
			A_Pitch = LookRot.Pitch;
			A_Yaw = LookRot.Yaw;
			if (GEngine)
			{
				FVector Loc = MyPlayer->GetActorLocation();

				FString Msg = FString::Printf(
					TEXT("Player Location: X=%.1f  Y=%.1f  Z=%.1f"),
					Loc.X, Loc.Y, Loc.Z
				);

				GEngine->AddOnScreenDebugMessage(
					-1,
					0.0f,            // 0초 = 매 프레임 갱신됨
					FColor::Green,
					Msg
				);
			}

		}
	}
	Speed = Pawn->GetVelocity().Size();

}
