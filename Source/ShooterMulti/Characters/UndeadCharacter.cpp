#include "UndeadCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "../Animations/UndeadCharacterAnim.h"
#include "../GameFramework/DeathMatchGS.h"
#include "Components/CapsuleComponent.h"

AUndeadCharacter::AUndeadCharacter()
{
	// Animation is set in UndeadCharacter_BP to fix build.
	//// Set Animations
	//ConstructorHelpers::FObjectFinder<UAnimBlueprint> AnimContainer(TEXT("AnimBlueprint'/Game/Blueprints/Animations/UndeadAnim_BP.UndeadAnim_BP'"));

	//if (AnimContainer.Succeeded())
	//	GetMesh()->SetAnimInstanceClass(AnimContainer.Object->GeneratedClass);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void AUndeadCharacter::BeginPlay()
{
	if(Team == ETeam::None)
		SetTeam(ETeam::AI);

	Super::BeginPlay();
	
	bHasPunched = false;
}

bool AUndeadCharacter::Punch()
{
	if (bHasPunched)
		return false;

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [this]() { bHasPunched = false; }, PunchCooldown, false);

	PlayPunchMontage();

	return bHasPunched = true;
}

void AUndeadCharacter::PlayPunchMontage()
{
	//Called on Server so don't need to use an RPC Server
	RPCMulticast_PlayPunchAnim();
}

void AUndeadCharacter::StartDisapear()
{
	Super::StartDisapear();
	
	ADeathMatchGS* GameState = Cast<ADeathMatchGS>(GetWorld()->GetGameState());
	//GameState->RemoveAI();
}

void AUndeadCharacter::Reset()
{
	ActivateRagdoll();

	Super::Reset();
}


void AUndeadCharacter::RPCMulticast_PlayPunchAnim_Implementation()
{
	USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(GetMesh());

	if (SkeletalMesh != nullptr)
	{
		UUndeadCharacterAnim* AnimInstance = Cast<UUndeadCharacterAnim>(SkeletalMesh->GetAnimInstance());
		if (AnimInstance)
			AnimInstance->PlayPunchMontage();
	}
}