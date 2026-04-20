// Copyright (c) 2025-2026 Betide Studio. All Rights Reserved.

#include "EGIK_CreateGroupTicket.h"

UEGIK_CreateGroupTicket* UEGIK_CreateGroupTicket::CreateGroupTicket(const FEGIK_CreateGroupTicketRequest& Request)
{
	UEGIK_CreateGroupTicket* Node = NewObject<UEGIK_CreateGroupTicket>();
	Node->Var_Request = Request;
	return Node;
}

FString UEGIK_CreateGroupTicket::GetEndpointURL() const
{
	FString BaseURL = Var_Request.MatchmakerUrl.IsEmpty()
		? UEGIKBlueprintFunctionLibrary::GetMatchmakingURL()
		: Var_Request.MatchmakerUrl;
	if (BaseURL.EndsWith(TEXT("/")))
	{
		BaseURL = BaseURL.LeftChop(1);
	}
	return BaseURL + TEXT("/group-tickets");
}

EEGIK_HttpVerb UEGIK_CreateGroupTicket::GetHTTPVerb() const
{
	return EEGIK_HttpVerb::POST;
}

FString UEGIK_CreateGroupTicket::GetAuthorizationHeader() const
{
	return Var_Request.AuthToken.IsEmpty()
		? UEGIKBlueprintFunctionLibrary::GetMatchmakingAuthToken()
		: Var_Request.AuthToken;
}

TSharedPtr<FJsonObject> UEGIK_CreateGroupTicket::BuildRequestBody() const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	TArray<TSharedPtr<FJsonValue>> MembersArray;
	for (const FEGIK_MemberTicket& Member : Var_Request.Members)
	{
		TSharedPtr<FJsonObject> MemberObject = MakeShareable(new FJsonObject);
		if(!Member.PlayerIp.IsEmpty())
		{
			MemberObject->SetStringField("player_ip", Member.PlayerIp);
		}
		const FString EffectiveProfile = Member.Profile.IsEmpty() ? Var_Request.ProfileId : Member.Profile;
		if (!EffectiveProfile.IsEmpty())
		{
			MemberObject->SetStringField(TEXT("profile"), EffectiveProfile);
		}
		TSharedPtr<FJsonObject> AttributesJsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Member.Attributes);
		if (FJsonSerializer::Deserialize(Reader, AttributesJsonObject) && AttributesJsonObject.IsValid())
		{
			MemberObject->SetObjectField(TEXT("attributes"), AttributesJsonObject);
		}
		else
		{
			UE_LOG(LogEdgegap, Error, TEXT("[Group] Failed to parse attributes JSON string."));
		}
		MembersArray.Add(MakeShareable(new FJsonValueObject(MemberObject)));
	}
	JsonObject->SetArrayField("player_tickets", MembersArray);

	return JsonObject;
}

void UEGIK_CreateGroupTicket::ProcessResponse(int32 HttpStatusCode, TSharedPtr<FJsonObject> JsonObject)
{
	TArray<FEGIK_MatchmakingResponse> ResponseArray;

	if (JsonObject.IsValid())
	{
		if(JsonObject->HasField(TEXT("player_tickets")))
		{
			const TArray<TSharedPtr<FJsonValue>> TicketsArray = JsonObject->GetArrayField(TEXT("player_tickets"));
			for (const TSharedPtr<FJsonValue>& TicketValue : TicketsArray)
			{
				TSharedPtr<FJsonObject> TicketObject = TicketValue->AsObject();
				if (TicketObject.IsValid())
				{
					FEGIK_MatchmakingResponse Response;
					if(TicketObject->HasField(TEXT("id")))
					{
						Response.TicketId = TicketObject->GetStringField(TEXT("id"));
					}
					if(TicketObject->HasField(TEXT("status")))
					{
						Response.Status = TicketObject->GetStringField(TEXT("status"));
					}
					if(TicketObject->HasField(TEXT("assignment")))
					{
						const TSharedPtr<FJsonObject>* AssignmentObject;
						if (TicketObject->TryGetObjectField(TEXT("assignment"), AssignmentObject))
						{
							Response.Assignment = FEGIK_AssignmentStruct(*AssignmentObject);
						}
						else
						{
							Response.Assignment = FEGIK_AssignmentStruct(TEXT("null"));
						}
					}
					if(TicketObject->HasField(TEXT("player_ip")))
					{
						Response.IP = TicketObject->GetStringField(TEXT("player_ip"));
					}
					if(TicketObject->HasField(TEXT("profile")))
					{
						Response.GameProfile = TicketObject->GetStringField(TEXT("profile"));
					}
					if(TicketObject->HasField(TEXT("group_id")))
					{
						Response.GroupId = TicketObject->GetStringField(TEXT("group_id"));
					}
					if (TicketObject->HasField(TEXT("team_id")))
					{
						Response.TeamId = TicketObject->GetStringField(TEXT("team_id"));
					}
					if (TicketObject->HasField(TEXT("match_id")))
					{
						Response.MatchId = TicketObject->GetStringField(TEXT("match_id"));
					}
					if(TicketObject->HasField(TEXT("created_at")))
					{
						FDateTime CreatedAt;
						FDateTime::ParseIso8601(*TicketObject->GetStringField(TEXT("created_at")), CreatedAt);
						Response.CreatedAt = CreatedAt;
					}
					ResponseArray.Add(Response);
				}
			}
		}
		OnSuccess.Broadcast(ResponseArray, FEGIK_ErrorStruct());
	}
	else
	{
		OnFailure.Broadcast(TArray<FEGIK_MatchmakingResponse>(), FEGIK_ErrorStruct(0, "Failed to parse JSON"));
	}
}

void UEGIK_CreateGroupTicket::HandleError(int32 ErrorCode, const FString& ErrorMessage)
{
	OnFailure.Broadcast(TArray<FEGIK_MatchmakingResponse>(), FEGIK_ErrorStruct(ErrorCode, ErrorMessage));
}

void UEGIK_CreateGroupTicket::HandleRateLimited(const FString& ResponseContent)
{
	OnRateLimited.Broadcast(TArray<FEGIK_MatchmakingResponse>(), FEGIK_ErrorStruct(429, ResponseContent));
}
