// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EGIK_AsyncRequestBase.h"
#include "EGIKBlueprintFunctionLibrary.h"
#include "EGIK_GetIpInformationBulk.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGetIpInformationBulkResponse, TArray<FEGIK_IpLookUpAddress>, IpAddresses, FEGIK_ErrorStruct, Error);

/**
 * Retrieves detailed information about multiple IP addresses.
 * POST /v1/ips/lookup
 */
UCLASS()
class EDGEGAPINTEGRATIONKIT_API UEGIK_GetIpInformationBulk : public UEGIK_AsyncRequestBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Edgegap Integration Kit | IpLookUp")
	static UEGIK_GetIpInformationBulk* GetIpInformationBulk(TArray<FString> IpAddresses);

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | IpLookUp")
	FGetIpInformationBulkResponse OnSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Edgegap Integration Kit | IpLookUp")
	FGetIpInformationBulkResponse OnFailure;

protected:
	virtual FString GetEndpointURL() const override;
	virtual EEGIK_HttpVerb GetHTTPVerb() const override;
	virtual TSharedPtr<FJsonObject> BuildRequestBody() const override;
	virtual void ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject) override;
	virtual void HandleError(int32 ErrorCode, const FString& ErrorMessage) override;
	virtual FString GetLogCategory() const override { return TEXT("IpLookUp"); }

private:
	TArray<FString> Var_IpAddresses;
};
