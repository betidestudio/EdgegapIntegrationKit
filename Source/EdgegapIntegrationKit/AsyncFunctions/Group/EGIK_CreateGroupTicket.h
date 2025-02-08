// Copyright (c) Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "EGIK_CreateGroupTicket.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_KeyValue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group")
	FString Key;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group")
	FString Value;
};

USTRUCT(BlueprintType)
struct FEGIK_MemberTicket
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group")
	FString PlayerIp;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group")
	FString Attributes;
};

USTRUCT(BlueprintType)
struct FEGIK_CreateGroupTicketRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group")
	FString ProfileId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group")
	TArray<FEGIK_MemberTicket> Members;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group")
	FString MatchmakerUrl;
	
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group")
	FString AuthToken;
};

USTRUCT(BlueprintType)
struct FEGIK_GroupMatchmakingResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString TicketId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString GameProfile;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString GroupId;
	
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString IP;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FEGIK_AssignmentStruct Assignment;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FDateTime CreatedAt;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Matchmaking")
	FString Status;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEGIK_CreateGroupMatchmakingTicketResponse, const TArray<FEGIK_GroupMatchmakingResponse>&, PlayerTickets, const FEGIK_ErrorStruct&, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_CreateGroupTicket : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Group")
	static UEGIK_CreateGroupTicket* CreateGroupTicket(const FEGIK_CreateGroupTicketRequest& Request);

	void OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg);
	virtual void Activate() override;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Group")
	FEGIK_CreateGroupMatchmakingTicketResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Group")
	FEGIK_CreateGroupMatchmakingTicketResponse OnFailure;

private:
	FEGIK_CreateGroupTicketRequest Var_Request;
	
	
};
