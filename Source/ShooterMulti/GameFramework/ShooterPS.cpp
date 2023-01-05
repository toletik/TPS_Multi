#include "ShooterPS.h"
#include "PlayerGI.h"
#include "../Characters/ShooterCharacter.h"
#include "DeathMatchGS.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5, FColor::White, text)

void AShooterPS::BeginPlay()
{
	Super::BeginPlay();

	ADeathMatchGS* GameState = GetWorld()->GetGameState<ADeathMatchGS>();
	GameState->OnResetAfterDelay.AddLambda([this]() { Reset(); });
}

void AShooterPS::CopyProperties(class APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (PlayerState)
	{
		AShooterPS* ShooterPlayerState = Cast<AShooterPS>(PlayerState);
		if (ShooterPlayerState)
		{
			ShooterPlayerState->NbKill = NbKill;
			ShooterPlayerState->NbKill = NbDeath;
			ShooterPlayerState->UserName = UserName;
			ShooterPlayerState->playerTeam = playerTeam;
		}
	}
}

void AShooterPS::OverrideWith(class APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);
	if (PlayerState)
	{
		AShooterPS* ShooterPlayerState = Cast<AShooterPS>(PlayerState);

		if (ShooterPlayerState)
		{
			NbKill = ShooterPlayerState->NbKill;
			NbDeath = ShooterPlayerState->NbDeath;
			UserName = ShooterPlayerState->UserName;
			playerTeam = ShooterPlayerState->playerTeam;
		}
	}
}

void AShooterPS::Reset()
{
	NbKill = 0;
	NbDeath = 0;
}

void AShooterPS::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AShooterPS, bIsReady);
	DOREPLIFETIME(AShooterPS, NbDeath);
	DOREPLIFETIME(AShooterPS, NbKill);
	DOREPLIFETIME(AShooterPS, UserName);
	DOREPLIFETIME(AShooterPS, playerTeam);
} 

void AShooterPS::RPCServer_PlayerIsReady_Implementation(FPlayerInfo playerInfo)
{
	bIsReady = true;
	UserName = playerInfo.UserName;
	playerTeam = playerInfo.TeamNum == 0 ? ETeam::Blue : ETeam::Red;

	if(isPlayersReady())
		GetWorld()->ServerTravel("Highrise?listen");
}

bool AShooterPS::isPlayersReady()
{
	ADeathMatchGS* gameState = Cast<ADeathMatchGS>(GetWorld()->GetGameState());
	for(int i = 0; i < gameState->PlayerArray.Num(); i++)
	{
		const AShooterPS* playerState = Cast<AShooterPS>(gameState->PlayerArray[i]);
		if(playerState && !playerState->bIsReady)
			return false;
	}

	return true;
}