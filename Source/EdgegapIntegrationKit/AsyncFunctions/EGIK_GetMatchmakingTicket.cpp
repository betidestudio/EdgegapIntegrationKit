// Copyright (c) 2024 Betide Studio. All Rights Reserved.


#include "EGIK_GetMatchmakingTicket.h"

#include "EGIKBlueprintFunctionLibrary.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

UEGIK_GetMatchmakingTicket* UEGIK_GetMatchmakingTicket::GetMatchmakingTicket(FEGIK_MatchmakingRequest MatchmakingRequest)
{
	UEGIK_GetMatchmakingTicket* BlueprintNode = NewObject<UEGIK_GetMatchmakingTicket>();
	BlueprintNode->Var_MatchmakingRequest = MatchmakingRequest;
	return BlueprintNode;
}

void UEGIK_GetMatchmakingTicket::OnResponseReceived(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	FEGIK_MatchmakingResponse Response;
	if(HttpResponse.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Response: %s"), *HttpResponse->GetContentAsString());
		if(EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(HttpResponse->GetContentAsString());
			if(FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				Response.TicketId = JsonObject->GetStringField(TEXT("id"));
				Response.GameProfile = JsonObject->GetStringField(TEXT("profile"));
				FDateTime::ParseIso8601(*JsonObject->GetStringField(TEXT("created_at")), Response.CreatedAt);
				const TSharedPtr<FJsonObject>* AssignmentObject;
				if (JsonObject->HasTypedField<EJson::Object>(TEXT("assignment")))
				{
					// The assignment field exists and is an object
					if (JsonObject->TryGetObjectField(TEXT("assignment"), AssignmentObject))
					{
						// Deserialize the assignment object
						Response.Assignment = FEGIK_AssignmentStruct(*AssignmentObject);
					}
					else
					{
						// The assignment field is either null or missing, so handle it as null
						Response.Assignment = FEGIK_AssignmentStruct("null");
					}
				}
				else
				{
					// The assignment field is either null or missing, so handle it as null
					Response.Assignment = FEGIK_AssignmentStruct("null");
				}
				if(JsonObject->HasField(TEXT("group_id")))
				{
					Response.GroupId = JsonObject->GetStringField(TEXT("group_id"));
				}
				if(JsonObject->HasField(TEXT("player_ip")))
				{
					Response.IP = JsonObject->GetStringField(TEXT("player_ip"));
				}
				if(JsonObject->HasField(TEXT("status")))
				{
					Response.Status = JsonObject->GetStringField(TEXT("status"));
				}
				OnSuccess.Broadcast(Response, FEGIK_ErrorStruct());
			}
			else
			{
				OnFailure.Broadcast(FEGIK_MatchmakingResponse(), FEGIK_ErrorStruct(0, "Failed to parse JSON"));
			}
		}
		else
		{
			OnFailure.Broadcast(FEGIK_MatchmakingResponse(), FEGIK_ErrorStruct(HttpResponse->GetResponseCode(), HttpResponse->GetContentAsString()));
		}
	}
	else
	{
		OnFailure.Broadcast(FEGIK_MatchmakingResponse(), FEGIK_ErrorStruct(0, "Failed to connect, likely the Matchmaker is down or the URL is incorrect or is not released"));
	}
}

void UEGIK_GetMatchmakingTicket::Activate()
{
	Super::Activate();
	Super::Activate();
	FHttpModule* Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetVerb("GET");
	Request->SetURL(Var_MatchmakingRequest.MatchmakingURL + "/tickets/" + Var_MatchmakingRequest.TicketId);
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("Authorization", Var_MatchmakingRequest.AuthToken);
	Request->OnProcessRequestComplete().BindUObject(this, &UEGIK_GetMatchmakingTicket::OnResponseReceived);
	if(!Request->ProcessRequest())
	{
		OnFailure.Broadcast(FEGIK_MatchmakingResponse(), FEGIK_ErrorStruct(0, "Failed to process request"));
		SetReadyToDestroy();
		MarkAsGarbage();
	}
}
