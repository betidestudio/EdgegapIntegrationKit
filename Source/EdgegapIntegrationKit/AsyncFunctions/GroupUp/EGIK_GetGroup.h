// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_GroupUpTypes.h"
#include "EGIK_GetGroup.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_GetGroupRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString GroupId;

	/** Optional override. Leave empty to use plugin settings/environment values. */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString MatchmakingURL;

	/** Optional override. Leave empty to use plugin settings/environment values. */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString AuthToken;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEGIK_GetGroupResponse, const FEGIK_GroupUpGroupResponse&, Response, const FEGIK_ErrorStruct&, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_GetGroup : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Group Up")
	static UEGIK_GetGroup* GetGroup(const FEGIK_GetGroupRequest& Request);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Group Up")
	FEGIK_GetGroupResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Group Up")
	FEGIK_GetGroupResponse OnFailure;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Group Up")
	FEGIK_GetGroupResponse OnRateLimited;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual FString GetAuthorizationHeader() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual void HandleRateLimited(const FString& ResponseContent) override;
	virtual FString GetLogCategory() const override { return TEXT("GroupUp"); }

private:
	FEGIK_GetGroupRequest Var_Request;
};
