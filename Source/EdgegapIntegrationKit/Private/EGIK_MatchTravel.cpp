// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#include "EGIKBlueprintFunctionLibrary.h"

#include "EGIK_AsyncRequestBase.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

namespace
{
	bool NormalizeMatchAddress(const FString& Address, FString& OutAddress, FString& OutError)
	{
		OutAddress = Address;
		OutAddress.TrimStartAndEndInline();

		if (OutAddress.StartsWith(TEXT("open "), ESearchCase::IgnoreCase))
		{
			OutAddress.RightChopInline(5);
			OutAddress.TrimStartAndEndInline();
		}

		if (OutAddress.IsEmpty())
		{
			OutError = TEXT("The match travel address is empty.");
			return false;
		}

		OutError.Reset();
		return true;
	}

	bool TravelPartyInternal(
		UWorld* World,
		const TArray<APlayerController*>& RequestedMembers,
		const FString& Address,
		FString& OutError,
		float LeaderTravelDelay,
		ETravelType TravelType)
	{
		if (!World)
		{
			OutError = TEXT("No game world is available for party travel.");
			return false;
		}

		if (World->GetNetMode() == NM_Client)
		{
			OutError = TEXT("Party travel must be called by the server/party authority.");
			return false;
		}

		FString NormalizedAddress;
		if (!NormalizeMatchAddress(Address, NormalizedAddress, OutError))
		{
			return false;
		}

		TArray<APlayerController*> Members;
		TSet<APlayerController*> UniqueMembers;
		for (APlayerController* PlayerController : RequestedMembers)
		{
			if (!IsValid(PlayerController))
			{
				OutError = TEXT("The party roster contains an invalid PlayerController.");
				return false;
			}

			if (PlayerController->GetWorld() != World)
			{
				OutError = FString::Printf(TEXT("PlayerController '%s' does not belong to the current lobby world."), *GetNameSafe(PlayerController));
				return false;
			}

			if (!UniqueMembers.Contains(PlayerController))
			{
				UniqueMembers.Add(PlayerController);
				Members.Add(PlayerController);
			}
		}

		if (Members.IsEmpty())
		{
			OutError = TEXT("The party roster is empty.");
			return false;
		}

		APlayerController* LocalLeader = nullptr;
		int32 RemoteMemberCount = 0;
		for (APlayerController* PlayerController : Members)
		{
			if (PlayerController->IsLocalController())
			{
				if (!LocalLeader)
				{
					LocalLeader = PlayerController;
				}
				continue;
			}

			++RemoteMemberCount;
		}

		if (World->GetNetMode() == NM_ListenServer && !LocalLeader)
		{
			OutError = TEXT("The listen-server leader is missing from the party roster.");
			return false;
		}

		// ClientTravel is already a reliable client RPC. Sending these calls before
		// the listen-server leader moves keeps the lobby connection alive long
		// enough for every remote member to receive its travel instruction.
		for (APlayerController* PlayerController : Members)
		{
			if (!PlayerController->IsLocalController())
			{
				PlayerController->ClientTravel(NormalizedAddress, TravelType, false);
			}
		}

		UE_LOG(LogEdgegap, Log, TEXT("[MatchTravel] Sent %d remote party member(s) to %s."), RemoteMemberCount, *NormalizedAddress);

		// Dedicated lobbies have no local leader. They stay up while the remote
		// party independently connects to the assigned match server.
		if (!LocalLeader)
		{
			OutError.Reset();
			return true;
		}

		const float SafeDelay = FMath::Max(0.0f, LeaderTravelDelay);
		if (RemoteMemberCount == 0 || SafeDelay <= 0.0f)
		{
			LocalLeader->ClientTravel(NormalizedAddress, TravelType, false);
			OutError.Reset();
			return true;
		}

		const TWeakObjectPtr<APlayerController> WeakLeader(LocalLeader);
		FTimerDelegate TravelDelegate = FTimerDelegate::CreateLambda(
			[WeakLeader, NormalizedAddress, TravelType]()
			{
				if (APlayerController* Leader = WeakLeader.Get())
				{
					Leader->ClientTravel(NormalizedAddress, TravelType, false);
				}
			});

		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(TimerHandle, TravelDelegate, SafeDelay, false);
		UE_LOG(LogEdgegap, Log, TEXT("[MatchTravel] Listen-server leader will travel in %.2f seconds."), SafeDelay);

		OutError.Reset();
		return true;
	}
}

bool UEGIKBlueprintFunctionLibrary::MakeMatchmakingAssignmentAddress(
	const FEGIK_AssignmentStruct& Assignment,
	FString& Address,
	FString& ErrorMessage,
	FName PortName,
	bool bPreferFQDN)
{
	Address.Reset();
	ErrorMessage.Reset();

	if (Assignment.bIsNullOrEmpty)
	{
		ErrorMessage = TEXT("The matchmaking assignment is empty.");
		return false;
	}

	FString Host = bPreferFQDN
		? (!Assignment.FQDN.IsEmpty() ? Assignment.FQDN : Assignment.PublicIP)
		: (!Assignment.PublicIP.IsEmpty() ? Assignment.PublicIP : Assignment.FQDN);
	Host.TrimStartAndEndInline();
	if (Host.IsEmpty())
	{
		ErrorMessage = TEXT("The matchmaking assignment contains neither an FQDN nor a public IP.");
		return false;
	}

	const FString RequestedPortName = PortName.IsNone() ? TEXT("gameport") : PortName.ToString();
	const FEGIK_PortStruct* SelectedPort = Assignment.Ports.Find(RequestedPortName);
	if (!SelectedPort)
	{
		for (const TPair<FString, FEGIK_PortStruct>& PortEntry : Assignment.Ports)
		{
			if (PortEntry.Key.Equals(RequestedPortName, ESearchCase::IgnoreCase))
			{
				SelectedPort = &PortEntry.Value;
				break;
			}
		}
	}

	if (!SelectedPort && RequestedPortName.Equals(TEXT("gameport"), ESearchCase::IgnoreCase))
	{
		SelectedPort = &Assignment.GamePort;
	}

	if (!SelectedPort || SelectedPort->ExternalPort <= 0)
	{
		ErrorMessage = FString::Printf(TEXT("The matchmaking assignment has no valid external port named '%s'."), *RequestedPortName);
		return false;
	}

	if (Host.Contains(TEXT(":")) && !Host.StartsWith(TEXT("[")))
	{
		Host = FString::Printf(TEXT("[%s]"), *Host);
	}

	Address = FString::Printf(TEXT("%s:%d"), *Host, SelectedPort->ExternalPort);
	return true;
}

bool UEGIKBlueprintFunctionLibrary::TravelPlayerToMatch(
	APlayerController* PlayerController,
	const FString& Address,
	FString& ErrorMessage,
	ETravelType TravelType)
{
	if (!IsValid(PlayerController))
	{
		ErrorMessage = TEXT("A valid PlayerController is required for match travel.");
		return false;
	}

	FString NormalizedAddress;
	if (!NormalizeMatchAddress(Address, NormalizedAddress, ErrorMessage))
	{
		return false;
	}

	PlayerController->ClientTravel(NormalizedAddress, TravelType, false);
	UE_LOG(LogEdgegap, Log, TEXT("[MatchTravel] Sent %s to %s."), *GetNameSafe(PlayerController), *NormalizedAddress);
	ErrorMessage.Reset();
	return true;
}

bool UEGIKBlueprintFunctionLibrary::TravelPlayerToMatchAssignment(
	APlayerController* PlayerController,
	const FEGIK_AssignmentStruct& Assignment,
	FString& ErrorMessage,
	FName PortName,
	bool bPreferFQDN,
	ETravelType TravelType)
{
	FString Address;
	if (!MakeMatchmakingAssignmentAddress(Assignment, Address, ErrorMessage, PortName, bPreferFQDN))
	{
		return false;
	}

	return TravelPlayerToMatch(PlayerController, Address, ErrorMessage, TravelType);
}

bool UEGIKBlueprintFunctionLibrary::TravelPartyToMatch(
	UObject* WorldContextObject,
	const TArray<APlayerController*>& PartyMembers,
	const FString& Address,
	FString& ErrorMessage,
	float LeaderTravelDelay,
	ETravelType TravelType)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return TravelPartyInternal(World, PartyMembers, Address, ErrorMessage, LeaderTravelDelay, TravelType);
}

bool UEGIKBlueprintFunctionLibrary::TravelPartyToMatchAssignment(
	UObject* WorldContextObject,
	const TArray<APlayerController*>& PartyMembers,
	const FEGIK_AssignmentStruct& Assignment,
	FString& ErrorMessage,
	FName PortName,
	bool bPreferFQDN,
	float LeaderTravelDelay,
	ETravelType TravelType)
{
	FString Address;
	if (!MakeMatchmakingAssignmentAddress(Assignment, Address, ErrorMessage, PortName, bPreferFQDN))
	{
		return false;
	}

	return TravelPartyToMatch(WorldContextObject, PartyMembers, Address, ErrorMessage, LeaderTravelDelay, TravelType);
}

bool UEGIKBlueprintFunctionLibrary::TravelAllConnectedPlayersToMatchAssignment(
	UObject* WorldContextObject,
	const FEGIK_AssignmentStruct& Assignment,
	FString& ErrorMessage,
	FName PortName,
	bool bPreferFQDN,
	float LeaderTravelDelay,
	ETravelType TravelType)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World)
	{
		ErrorMessage = TEXT("No game world is available for party travel.");
		return false;
	}

	TArray<APlayerController*> ConnectedPlayers;
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (APlayerController* PlayerController = Iterator->Get())
		{
			ConnectedPlayers.Add(PlayerController);
		}
	}

	return TravelPartyToMatchAssignment(
		WorldContextObject,
		ConnectedPlayers,
		Assignment,
		ErrorMessage,
		PortName,
		bPreferFQDN,
		LeaderTravelDelay,
		TravelType);
}
