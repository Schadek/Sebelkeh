// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_DELEGATE_RetVal_OneParam(FString, DescriptionParserCallback, const FString&);
DECLARE_DELEGATE_RetVal_TwoParams(FString, DescriptionParserArrayCallback, const FString&, const TArray<int32>&);

class COURTOFSEBELKEH_API DescriptionParser
{

public:

	DescriptionParser();
	DescriptionParser(UObject* DescribedObject);

	FText ParseDescription(const FText& Description) const;
	FString ParseKeyword(const FString& Keyword, const FString& Indexer) const;

	void AddDescribedObject(UObject* NewDescribedObject) { DescribedObjects.Add(NewDescribedObject); }
	TArray<UObject*> GetDescribedObjects() const { return DescribedObjects; }

	void RegisterKeyword(const FString& Keyword, const DescriptionParserCallback& Callback);
	void UnregisterKeyword(const FString& Keyword);

	void RegisterArrayIndexer(const FString& Indexer, const DescriptionParserArrayCallback& Callback);
	void UnregisterArrayIndexer(const FString& Indexer);

protected:

	TArray<UObject*> DescribedObjects;
	TMap<FString, DescriptionParserCallback> KeywordMap;
	TMap<FString, DescriptionParserArrayCallback> IndexerMap;

};