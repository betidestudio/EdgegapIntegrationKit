// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIK_GroupUpTypes.h"
#include "EGIK_MatchmakingAttributes.h"
#include "EGIK_CreateGroupMember.generated.h"

USTRUCT(BlueprintType)
struct FEGIK_CreateGroupMemberRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString GroupId;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString Profile;

	/** Recommended: type-safe attributes that require no JSON syntax. */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FEGIK_MatchmakingAttributes StructuredAttributes;

	/** Advanced/backward-compatible raw JSON object. Do not set together with StructuredAttributes. */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString Attributes;

	/** Best-effort runtime comparison against the live Matchmaker profile schema. Logs only and never blocks this request. */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up | Debug")
	bool bLogProfileDebugInfo = false;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString PlayerIp;

	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	bool bIsReady = false;

	/** Optional override. Leave empty to use plugin settings/environment values. */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString MatchmakingURL;

	/** Optional override. Leave empty to use plugin settings/environment values. */
	UPROPERTY(BlueprintReadWrite, Category = "Edgegap Integration Kit | Group Up")
	FString AuthToken;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEGIK_CreateGroupMemberResponse, const FEGIK_GroupUpMemberResponse&, Response, const FEGIK_ErrorStruct&, Error);

UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_CreateGroupMember : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | Group Up")
	static UEGIK_CreateGroupMember* CreateGroupMember(const FEGIK_CreateGroupMemberRequest& Request);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Group Up")
	FEGIK_CreateGroupMemberResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Group Up")
	FEGIK_CreateGroupMemberResponse OnFailure;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | Group Up")
	FEGIK_CreateGroupMemberResponse OnRateLimited;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual FString GetAuthorizationHeader() const override;
	virtual bool ValidateRequest(FString& OutErrorMessage) const override;
	virtual TSharedPtr<FJsonObject> BuildRequestBody() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual void HandleRateLimited(const FString& ResponseContent) override;
	virtual FString GetLogCategory() const override { return TEXT("GroupUp"); }

private:
	FEGIK_CreateGroupMemberRequest Var_Request;
};
