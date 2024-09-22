// Copyright (c) 2024 Betide Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNodeUtils.h" // for FNodeTextCache
#include "EdGraphSchema_K2.h"
#include "EdGraphUtilities.h"
#include "EditorCategoryUtils.h"
#include "KismetCompiler.h"
#include "Runtime/Launch/Resources/Version.h"
#include "K2Node_MakeAttributes.generated.h"

UENUM(BlueprintType)
enum class EEGIK_JsonType : uint8
{
	// JSON_Null UMETA(DisplayName = "Null"),
	JSON_Bool UMETA(DisplayName = "Boolean"),
	JSON_Number UMETA(DisplayName = "Number"),
	JSON_String UMETA(DisplayName = "String"),
	JSON_Object UMETA(DisplayName = "Object")
};

USTRUCT(BlueprintType)
struct FEGIK_NamedType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = NamedType)
	FName Name;

	UPROPERTY(EditAnywhere, Category = NamedType)
	EEGIK_JsonType Type;

	UPROPERTY(EditAnywhere, Category = NamedType)
	bool bIsArray;

	FEGIK_NamedType()
		: Type(EEGIK_JsonType::JSON_String)
		, bIsArray(false)
	{
	}
};

UCLASS(BlueprintType, Blueprintable)
class EDGEGAPEDITOR_API UK2Node_MakeAttributes : public UK2Node
{
	GENERATED_BODY()
	
	// Overrided from UEdGraphNode.
	virtual void AllocateDefaultPins() override;
	virtual FText GetTooltipText() const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;

	// Overrided from UK2Node.
	virtual bool IsNodePure() const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Function to add more input pins dynamically
	void AddInputPin();
    
	// Function to remove input pins
	void RemoveInputPin();

public:
	UPROPERTY(EditAnywhere, Category = PinOptions)
	TArray<FEGIK_NamedType> Outputs;

};
