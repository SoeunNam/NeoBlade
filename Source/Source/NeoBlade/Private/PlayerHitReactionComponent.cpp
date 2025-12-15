#include "PlayerHitReactionComponent.h"
#include "PlayerAnim.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"

UPlayerHitReactionComponent::UPlayerHitReactionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    PlayerAnim = nullptr;
}

void UPlayerHitReactionComponent::BeginPlay()
{
    Super::BeginPlay();

    // 소유자 Actor에서 AnimInstance 찾아서 캐싱
    if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
    {
        PlayerAnim = Cast<UPlayerAnim>(OwnerChar->GetMesh()->GetAnimInstance());
    }
}

void UPlayerHitReactionComponent::PlayHitReaction()
{


    if (!PlayerAnim)
    {
        // 혹시 캐싱 실패했을 경우 한 번 더 시도
        if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
        {
            PlayerAnim = Cast<UPlayerAnim>(OwnerChar->GetMesh()->GetAnimInstance());
            UE_LOG(LogTemp, Error, TEXT("[HitReaction] PlayerAnim recached: %s"),
                PlayerAnim ? TEXT("OK") : TEXT("FAILED")); PlayerAnim = Cast<UPlayerAnim>(OwnerChar->GetMesh()->GetAnimInstance());
        }
    }

    if (!PlayerAnim)
    {
        UE_LOG(LogTemp, Error, TEXT("[HitReaction] PlayerAnim is NULL!"));
        return;
    }

    // ===============================
    // ★ 애님 인스턴스에게 피격 신호 전달
    // ===============================

    // 1) AnimBP 이벤트 호출
    PlayerAnim->OnHitReaction();  // AnimBP에서 구현할 Event

    // 2) 또는 몽타주 직접 실행
    if (PlayerAnim->HitReactMontage)
    {
        PlayerAnim->Montage_Play(PlayerAnim->HitReactMontage, 1.0f);
    }

    // 3) AnimBP 변수 세팅 방식도 가능
    // PlayerAnim->bHitStun = true;
}

void UPlayerHitReactionComponent::PlayGuardHitReaction(AActor* Attacker)
{
    if (!PlayerAnim) return;

    UE_LOG(LogTemp, Warning, TEXT("[GuardHitReaction] Guard Hit Reaction"));

    if (PlayerAnim->GuardHitMontage)   // 에디터에서 지정
    {
        PlayerAnim->PlayGuardHitMontage();
    }
}
