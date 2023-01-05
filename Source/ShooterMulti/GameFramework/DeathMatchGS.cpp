#include "DeathMatchGS.h"
#include "ShooterPS.h"
#include "TimerManager.h"
#include "DeathMatchGM.h"
#include "../Characters/ShooterCharacter.h"
#include "../LD/Pickup.h"
#include "../Controllers/ShooterController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EngineUtils.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5, FColor::White, text)

void ADeathMatchGS::BeginPlay()
{
	Super::BeginPlay();

	OnTeamWin.AddLambda([this](ETeam Team) { ShowTeamWinHUD(Team); });

	OnGameRestart.AddLambda([this]() { Reset(); });

	GameMode = Cast<ADeathMatchGM>(AuthorityGameMode);

	if (GetLocalRole() == ENetRole::ROLE_Authority)
	{
		check(GameMode && "GameMode nullptr: Cast as ADeathMatchGM failed.");

		CurrentTime = GameMode->GameTime;
		GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ADeathMatchGS::AdvanceTimer, 1.0f, true);
	}


}

void ADeathMatchGS::AdvanceTimer()
{
	--CurrentTime;
	
	if (CurrentTime <= 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
		if (RedTeamScore < BlueTeamScore)
			UpdateEndHud(ETeam::Blue);
		else if (RedTeamScore > BlueTeamScore)
			UpdateEndHud(ETeam::Red);
		else
			UpdateEndHud(ETeam::None);
	}
}

void ADeathMatchGS::AddScore(ETeam Team)
{
	if (Team == ETeam::Red && ++RedTeamScore == GameMode->MaxKill)
		UpdateEndHud(ETeam::Red);
	else if (Team == ETeam::Blue && ++BlueTeamScore == GameMode->MaxKill)
		UpdateEndHud(ETeam::Blue);
}

void ADeathMatchGS::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	OnPlayerNum.Broadcast(this);
}

void ADeathMatchGS::RemovePlayerState(APlayerState* PlayerState)
{
	OnPlayerNum.Broadcast(this);

	Super::RemovePlayerState(PlayerState);
}

bool ADeathMatchGS::CanAddAI()
{
	if (GetLocalRole() == ENetRole::ROLE_Authority)
		return Cast<ADeathMatchGM>(GetWorld()->GetAuthGameMode())->MaxAIPerPlayer* PlayerArray.Num() > CurrentAICount;

	return false;
}

void ADeathMatchGS::AddAI()
{
	CurrentAICount++;
	print(((GetLocalRole() == ROLE_Authority) ? "server " : "client ") + GetFName().ToString() + " Add AICount " + FString::FromInt(CurrentAICount));
}

void ADeathMatchGS::RemoveAI()
{
	CurrentAICount--;
	print(((GetLocalRole() == ROLE_Authority) ? "server " : "client ") + GetFName().ToString() + " Remove AICount " + FString::FromInt(CurrentAICount));
}

int ADeathMatchGS::GetNbplayer()
{
	return PlayerArray.Num();
}

int ADeathMatchGS::NewFrequency(int Sec)
{
	return Sec / sqrt(0.61 * PlayerArray.Num());
}

void ADeathMatchGS::UpdateEndHud(ETeam Team)
{
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
	OnTeamWin.Broadcast(Team);
}

void ADeathMatchGS::Reset()
{
	TArray<AActor*> Resetables;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UResetable::StaticClass(), Resetables);

	for (auto& res : Resetables)
		Cast<IResetable>(res)->Reset();
}

void ADeathMatchGS::ResetAfterDelay()
{
	CurrentTime = GameMode->GameTime;
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, this, &ADeathMatchGS::AdvanceTimer, 1.0f, true);

	RedTeamScore = 0;
	BlueTeamScore = 0;
	CurrentAICount = 0;

	OnResetAfterDelay.Broadcast();
}

void ADeathMatchGS::EndGameTrigg()
{
	OnGameRestart.Broadcast();
}

//Replicate
void ADeathMatchGS::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADeathMatchGS, RedTeamScore);
	DOREPLIFETIME(ADeathMatchGS, BlueTeamScore);
	DOREPLIFETIME(ADeathMatchGS, CurrentTime);
	DOREPLIFETIME(ADeathMatchGS, CurrentAICount);

}
