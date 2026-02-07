// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_BulkStopDeployments.h"

UEGIK_BulkStopDeployments* UEGIK_BulkStopDeployments::BulkStopDeployments(const TArray<FString>& RequestIds)
{
	UEGIK_BulkStopDeployments* Node = NewObject<UEGIK_BulkStopDeployments>();
	Node->Var_RequestIds = RequestIds;
	return Node;
}

FString UEGIK_BulkStopDeployments::GetEndpointURL() const
{
	return TEXT("https://api.edgegap.com/v1/deployments/bulk-stop");
}

EEGIK_HttpVerb UEGIK_BulkStopDeployments::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

TSharedPtr<FJsonObject> UEGIK_BulkStopDeployments::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetArrayField(TEXT("request_ids"), StringArrayToJsonArray(Var_RequestIds));
	return JsonObject;
}

void UEGIK_BulkStopDeployments::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	FEGIK_BulkStopDeploymentsResponse Response;

	if (JsonObject.IsValid())
	{
		// Parse results array if present
		if (JsonObject->HasField(TEXT("results")))
		{
			const TArray<TSharedPtr<FJsonValue>>* ResultsArray;
			if (JsonObject->TryGetArrayField(TEXT("results"), ResultsArray))
			{
				for (const auto& ResultValue : *ResultsArray)
				{
					TSharedPtr<FJsonObject> ResultObject = ResultValue->AsObject();
					if (ResultObject.IsValid())
					{
						FEGIK_BulkStopResult Result;
						Result.RequestId = ResultObject->GetStringField(TEXT("request_id"));
						Result.bSuccess = ResultObject->GetBoolField(TEXT("success"));
						Result.Message = ResultObject->GetStringField(TEXT("message"));
						Response.Results.Add(Result);

						if (Result.bSuccess)
						{
							Response.SuccessCount++;
						}
						else
						{
							Response.FailureCount++;
						}
					}
				}
			}
		}

		// Some APIs return success/failure counts directly
		if (JsonObject->HasField(TEXT("success_count")))
		{
			Response.SuccessCount = JsonObject->GetIntegerField(TEXT("success_count"));
		}
		if (JsonObject->HasField(TEXT("failure_count")))
		{
			Response.FailureCount = JsonObject->GetIntegerField(TEXT("failure_count"));
		}
	}

	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_BulkStopDeployments::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_BulkStopDeploymentsResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
