// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_GetContainerLogs.h"

UEGIK_GetContainerLogs* UEGIK_GetContainerLogs::GetContainerLogs(const FString& RequestId)
{
	UEGIK_GetContainerLogs* Node = NewObject<UEGIK_GetContainerLogs>();
	Node->Var_RequestId = RequestId;
	return Node;
}

FString UEGIK_GetContainerLogs::GetEndpointURL() const
{
	return FString::Printf(TEXT("https://api.edgegap.com/v1/deployment/%s/container-logs"), *Var_RequestId);
}

EEGIK_HttpVerb UEGIK_GetContainerLogs::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::GET;
}

void UEGIK_GetContainerLogs::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	FEGIK_ContainerLogsResponse Response;
	Response.RequestId = Var_RequestId;

	if (JsonObject.IsValid())
	{
		// Try to get raw logs string
		if (JsonObject->HasField(TEXT("logs")))
		{
			Response.RawLogs = JsonObject->GetStringField(TEXT("logs"));
		}

		// Try to get encoding
		if (JsonObject->HasField(TEXT("encoding")))
		{
			Response.Encoding = JsonObject->GetStringField(TEXT("encoding"));
		}

		// Try to parse structured logs array if present
		if (JsonObject->HasField(TEXT("entries")))
		{
			const TArray<TSharedPtr<FJsonValue>>* EntriesArray;
			if (JsonObject->TryGetArrayField(TEXT("entries"), EntriesArray))
			{
				for (const auto& EntryValue : *EntriesArray)
				{
					TSharedPtr<FJsonObject> EntryObject = EntryValue->AsObject();
					if (EntryObject.IsValid())
					{
						FEGIK_ContainerLogEntry Entry;
						Entry.Timestamp = EntryObject->GetStringField(TEXT("timestamp"));
						Entry.Message = EntryObject->GetStringField(TEXT("message"));
						Entry.Level = EntryObject->GetStringField(TEXT("level"));
						Response.Logs.Add(Entry);
					}
				}
			}
		}
	}

	OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
}

void UEGIK_GetContainerLogs::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(FEGIK_ContainerLogsResponse(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}
