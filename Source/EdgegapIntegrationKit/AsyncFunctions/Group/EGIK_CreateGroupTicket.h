// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
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


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEGIK_CreateGroupMatchmakingTicketResponse, const TArray<FEGIK_MatchmakingResponse>&, PlayerTickets, const FEGIK_ErrorStruct&, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_CreateGroupTicket : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Group")
	static UEGIK_CreateGroupTicket* CreateGroupTicket(const FEGIK_CreateGroupTicketRequest& Request);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Group")
	FEGIK_CreateGroupMatchmakingTicketResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Group")
	FEGIK_CreateGroupMatchmakingTicketResponse OnFailure;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Group")
	FEGIK_CreateGroupMatchmakingTicketResponse OnRateLimited;

protected:
	//~ Begin UEGIK_AsyncRequestBase Interface
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual FString GetAuthorizationHeader() const override;
	virtual TSharedPtr<FJsonObject> BuildRequestBody() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual void HandleRateLimited(const FString& ResponseContent) override;
	virtual FString GetLogCategory() const override { return TEXT("Group"); }
	//~ End UEGIK_AsyncRequestBase Interface

private:
	FEGIK_CreateGroupTicketRequest Var_Request;
};
