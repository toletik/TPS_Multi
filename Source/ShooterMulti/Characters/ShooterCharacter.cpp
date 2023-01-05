#include "ShooterCharacter.h"
#include "../Animations/ShooterCharacterAnim.h"
#include "../GameFramework/PlayerGI.h"
#include "../LD/EnemySpawnerButton.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "Animation/AnimBlueprint.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "../GameFramework/DeathMatchGS.h"
#include "../GameFramework/ShooterPS.h"


#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5, FColor::White, text)

AShooterCharacter::AShooterCharacter()
{
	DisapearingDelay = 1.5f;

	// Create Weapon
	Weapon = CreateDefaultSubobject<UWeaponComponent>("Rifle");

	ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshContainer(TEXT("SkeletalMesh'/Game/Weapons/Rifle.Rifle'"));
	if (MeshContainer.Succeeded())
		Weapon->SetSkeletalMesh(MeshContainer.Object);

	Weapon->SetRelativeLocation(FVector(1.0f, 4.0f, -2.0f));
	Weapon->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Weapon->SetupAttachment(GetMesh(), "hand_r");

	// Create Camera
	Camera = CreateDefaultSubobject<UPlayerCameraComponent>("PlayerCamera");
	Camera->SetupAttachment(RootComponent);

	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
}

EShooterCharacterState AShooterCharacter::GetState() const
{
	return State;
}

void AShooterCharacter::SetState(EShooterCharacterState InState)
{
	RPCServer_SetState(InState);
}

UWeaponComponent* AShooterCharacter::GetWeaponComponent()
{
	return Weapon;
}

UPlayerCameraComponent* AShooterCharacter::GetCameraComponent()
{
	return Camera;
}

void AShooterCharacter::UpdateAimOffsets(float Pitch, float Yaw)
{
	RPCServer_UpdateAimOffsets(Pitch, Yaw);
}

void AShooterCharacter::InitPlayer()
{
	AShooterPS* playerState = Cast<AShooterPS>(GetPlayerState());

	if(playerState)
	{
		Team = playerState->playerTeam;
		InitTeamColor(playerState->playerTeam);
	}
}

void AShooterCharacter::InitTeamColor(ETeam InTeam)
{
	RPCServer_SetTeam(InTeam);
	OnTeamSwitch.Broadcast();
}

void AShooterCharacter::Invincibility(float Duration)
{
	Health = 100000;
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, [this]() { Health = MaxHealth; }, Duration, false);

	InvincibilityFX(Duration);
}

void AShooterCharacter::BeginPlay()
{
	RPCServer_SetName();
	OnTeamSwitch.AddLambda([this]() { RefreshTeamHUD(Team); });
	
	Super::BeginPlay();
	
	RunSpeed = GetCharacterMovement()->MaxWalkSpeed;

	if (GetLocalRole() == ENetRole::ROLE_Authority)
		Invincibility(Cast<ADeathMatchGM>(GetWorld()->GetAuthGameMode())->InvincibilityTime);
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead())
		return;

	if (bIsShooting)
	{
		RPCServer_SetCameraTransform(GetCameraComponent()->GetCameraHandle()->GetComponentTransform());
		if(GetLocalRole() != ENetRole::ROLE_Authority && !Weapon->Shot(_cameraTransform))
			StartReload();
	}

	// Anim aim offsets
	FRotator LookRotation = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation());

	UpdateAimOffsets(UKismetMathLibrary::ClampAngle(LookRotation.Pitch, -90.f, 90.f), UKismetMathLibrary::ClampAngle(LookRotation.Yaw, -90.f, 90.f));

	Camera->ShakeCamera(uint8(State), GetLastMovementInputVector().Size());
}

void AShooterCharacter::StartSprint()
{
	if (bIsShooting)
		EndShoot();

	if (State == EShooterCharacterState::Reload)
		AbortReload();
	else if (State == EShooterCharacterState::Aim)
		EndAim();

	if (State != EShooterCharacterState::IdleRun && State != EShooterCharacterState::Jump)
		return;

	if (State == EShooterCharacterState::Jump)
		RPCServer_SetPrevState(EShooterCharacterState::Sprint);
	else
		SetState(EShooterCharacterState::Sprint);

	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AShooterCharacter::EndSprint()
{
	if (State != EShooterCharacterState::Sprint && State != EShooterCharacterState::Jump)
		return;

	if (State == EShooterCharacterState::Jump)
		RPCServer_SetPrevState(EShooterCharacterState::IdleRun);
	else
		SetState(EShooterCharacterState::IdleRun);

	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void AShooterCharacter::StartJump()
{
	if (bIsShooting)
		EndShoot();

	if (State == EShooterCharacterState::Aim)
		EndAim();
	else if (State == EShooterCharacterState::Reload)
		AbortReload();

	if (CanJump() && (State == EShooterCharacterState::IdleRun || State == EShooterCharacterState::Sprint))
	{
		SetState(EShooterCharacterState::Jump);
		Jump();
	}
}

void AShooterCharacter::EndJump()
{
	if (State != EShooterCharacterState::Jump && State != EShooterCharacterState::Falling)
		return;

	SetState(EShooterCharacterState::IdleRun);
	StopJumping();
}

void AShooterCharacter::StartAim()
{
	if (State != EShooterCharacterState::IdleRun)
		return;
	
	SetState(EShooterCharacterState::Aim);

	GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed;

	Camera->SwitchToAimCamera();
}

void AShooterCharacter::EndAim()
{
	if (State != EShooterCharacterState::Aim)
		return;

	SetState(PrevState);
	
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	
	Camera->SwitchToWalkCamera();
}

void AShooterCharacter::StartShoot()
{
	if (State == EShooterCharacterState::IdleRun || State == EShooterCharacterState::Aim)
		RPCServer_Shoot(true);
}

void AShooterCharacter::EndShoot()
{
	RPCServer_Shoot(false);
}

void AShooterCharacter::StartReload()
{
	if (Weapon && Weapon->AmmoCount > 0 && Weapon->WeaponMagazineSize > Weapon->LoadedAmmo)
	{
		if (State == EShooterCharacterState::Aim)
			EndAim();
		else if (bIsShooting)
			RPCServer_Shoot(false);

		if (State != EShooterCharacterState::IdleRun)
			return;

		SetState(EShooterCharacterState::Reload);
		
		GetCharacterMovement()->MaxWalkSpeed = ReloadWalkSpeed;
	}
}

void AShooterCharacter::EndReload()
{
	if (State != EShooterCharacterState::Reload)
		return;

	SetState(EShooterCharacterState::IdleRun);
	
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;

	if(Weapon)
		Weapon->Reload();
}
void AShooterCharacter::AbortReload()
{
	if (State != EShooterCharacterState::Reload)
		return;

	SetState(EShooterCharacterState::IdleRun);

	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void AShooterCharacter::Falling()
{
	Super::Falling();

	if (State == EShooterCharacterState::Jump)
		return;

	if (bIsShooting)
		EndShoot();

	if (State == EShooterCharacterState::Aim)
		EndAim();
	else if (State == EShooterCharacterState::Reload)
		AbortReload();

	SetState(EShooterCharacterState::Falling);
}

void AShooterCharacter::PushButton()
{
	if (bIsShooting)
		RPCServer_Shoot(false);
	else if (State == EShooterCharacterState::Reload)
		AbortReload();

	if (State != EShooterCharacterState::IdleRun)
		return;

	SetState(EShooterCharacterState::PushButton);
	PlayPushButtonAnim();
}

void AShooterCharacter::InflictPushButton()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, TSubclassOf<AEnemySpawnerButton>());

	if (OverlappingActors.Num() > 0)
	{
		AEnemySpawnerButton* Button = Cast<AEnemySpawnerButton>(OverlappingActors[0]);

		if (Button)
			Button->Activate(Team);
	}
}

void AShooterCharacter::PlayPushButtonAnim()
{
	RPCServer_PlayPushButtonAnim();
}

void AShooterCharacter::Punch()
{
	if (bIsShooting)
		RPCServer_Shoot(false);
	else if (State == EShooterCharacterState::Reload)
		AbortReload();

	if (State != EShooterCharacterState::IdleRun)
		return;

	SetState(EShooterCharacterState::Punch);
	PlayPunchAnim();
}

void AShooterCharacter::PlayPunchAnim()
{
	RPCServer_PlayPunchAnim();
}

void AShooterCharacter::StartDisapear()
{
	RPCServer_AddNumberOfDeath(Cast<AShooterPS>(GetPlayerState()));

	Super::StartDisapear();

	
	FTimerHandle Handle1;
	GetWorld()->GetTimerManager().SetTimer(Handle1, [this]() { Weapon->SetVisibility(false, true); }, 3.5f, false);

	if (Controller)
	{
		APlayerController* PlayerControler = Cast<APlayerController>(Controller);
		PlayerControler->DisableInput(PlayerControler);
		
		FTimerHandle Handle2;
		GetWorld()->GetTimerManager().SetTimer(Handle2, [PlayerControler]() { PlayerControler->EnableInput(PlayerControler); }, 5.0f, false);
	}
}

void AShooterCharacter::FinishDisapear()
{
	APlayerController* PlayerController = Cast<APlayerController>(Controller);

	Super::FinishDisapear();

	if(GetLocalRole() == ROLE_Authority)
		Cast<ADeathMatchGM>(GetWorld()->GetAuthGameMode())->Respawn(PlayerController);
}

//Replicate
void AShooterCharacter::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterCharacter, State);
	DOREPLIFETIME(AShooterCharacter, PrevState);
	DOREPLIFETIME(AShooterCharacter, AimPitch);
	DOREPLIFETIME(AShooterCharacter, AimYaw);
	DOREPLIFETIME(AShooterCharacter, bIsShooting);
	DOREPLIFETIME(AShooterCharacter, _cameraTransform);
	DOREPLIFETIME(AShooterCharacter, name);

}	

//RPC
void AShooterCharacter::RPCServer_SetPrevState_Implementation(EShooterCharacterState InState)
{
	PrevState = InState;
}

void AShooterCharacter::RPCServer_SetState_Implementation(EShooterCharacterState InState)
{
	PrevState = State;
	State = InState;
}

void AShooterCharacter::RPCServer_UpdateAimOffsets_Implementation(float Pitch, float Yaw)
{
	AimPitch = Pitch;
	AimYaw = Yaw;
}

void AShooterCharacter::RPCServer_Shoot_Implementation(bool isShooting)
{
	bIsShooting = isShooting;
}

void AShooterCharacter::RPCServer_PlayPunchAnim_Implementation()
{
	RPCMulticast_PlayPunchAnim();
}
void AShooterCharacter::RPCMulticast_PlayPunchAnim_Implementation()
{
	Cast<UShooterCharacterAnim>(GetMesh()->GetAnimInstance())->PlayPunchMontage();
}

void AShooterCharacter::RPCServer_PlayPushButtonAnim_Implementation()
{
	RPCMulticast_PlayPushButtonAnim();
}
void AShooterCharacter::RPCMulticast_PlayPushButtonAnim_Implementation()
{
	Cast<UShooterCharacterAnim>(GetMesh()->GetAnimInstance())->PlayPushButtonMontage();
}

void AShooterCharacter::RPCServer_SetCameraTransform_Implementation(FTransform cameraTransform)
{
	this->_cameraTransform = cameraTransform;
}

void AShooterCharacter::RPCServer_SetTeam_Implementation(ETeam InTeam)
{
	Team = InTeam;
}

void AShooterCharacter::RPCServer_AddScore_Implementation(AShooterPS* shooterPlayerState)
{
	shooterPlayerState->NbKill++;

	if (GetWorld() && GetWorld()->GetGameState<ADeathMatchGS>())
		GetWorld()->GetGameState<ADeathMatchGS>()->AddScore(Team);

}

void AShooterCharacter::RPCServer_AddNumberOfDeath_Implementation(AShooterPS* shooterPlayerState)
{
	shooterPlayerState->NbDeath++;
}

void AShooterCharacter::RPCServer_SetName_Implementation()
{
	name = "Player number : " + FString::FromInt(FMath::RandRange(0, 10000));
}

void AShooterCharacter::RPCServer_RemoveAI_Implementation()
{
	if (GetWorld() && GetWorld()->GetGameState<ADeathMatchGS>())
		GetWorld()->GetGameState<ADeathMatchGS>()->RemoveAI();
}