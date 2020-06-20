// Fill out your copyright notice in the Description page of Project Settings.


#include "DescriptionParser.h"
#include "Misc/DefaultValueHelper.h"

DescriptionParser::DescriptionParser()
{ }

DescriptionParser::DescriptionParser(UObject* DescribedObject)
{
	AddDescribedObject(DescribedObject);
}

FText DescriptionParser::ParseDescription(const FText& Description) const
{
	const FString RawDesc = Description.ToString();
	FString FormattedDesc;

	int32 Index = 0;
	while (Index != RawDesc.Len())
	{
		switch (RawDesc[Index])
		{
		case '{':
		{
			const int32 ClosingBracketIndex = RawDesc.Find("}", ESearchCase::IgnoreCase, ESearchDir::FromStart, Index);
			if (ClosingBracketIndex > 0)
			{
				const int32 ArrayIndexerBracket = RawDesc.Find("[", ESearchCase::IgnoreCase, ESearchDir::FromStart, Index);
				if (ArrayIndexerBracket > 0 && ArrayIndexerBracket < ClosingBracketIndex)
				{
					FString KeywordName = RawDesc.Mid(Index + 1, ArrayIndexerBracket - Index - 1);
					FString IndexerName = RawDesc.Mid(ArrayIndexerBracket + 1, ClosingBracketIndex - ArrayIndexerBracket - 2);
					FormattedDesc.Append(ParseKeyword(KeywordName, IndexerName));
				}
				else
				{
					FString KeywordName = RawDesc.Mid(Index + 1, ClosingBracketIndex - Index - 1);
					FormattedDesc.Append(ParseKeyword(KeywordName, ""));
				}

				Index = ClosingBracketIndex;
			}
			else
			{
				FormattedDesc = "<INVALID DESCRIPTION>";
			}
			break;
		}
		default:
			FormattedDesc.AppendChar(RawDesc[Index]);
		}

		Index++;
	}

	return FText::FromString(FormattedDesc);
}

FString DescriptionParser::ParseKeyword(const FString& Keyword, const FString& Indexer) const
{
	if (const DescriptionParserCallback* Callback = KeywordMap.Find(Keyword))
	{
		if (Callback->IsBound())
		{
			return Callback->Execute(Keyword);
		}
	}

	for (UObject* DescribedObject : DescribedObjects)
	{
		if (FProperty* Property = DescribedObject->GetClass()->FindPropertyByName(*Keyword))
		{
			const void* ThisPtr = Property->ContainerPtrToValuePtr<void>(DescribedObject);
			if (const FIntProperty* IntProperty = CastField<const FIntProperty>(Property))
			{
				int32 ReadValue = IntProperty->GetSignedIntPropertyValue(ThisPtr);
				return FString::FromInt(ReadValue);
			}
			else if (const FFloatProperty* FloatProperty = CastField<const FFloatProperty>(Property))
			{
				float ReadValue = FloatProperty->GetFloatingPointPropertyValue(ThisPtr);
				return FString::SanitizeFloat(ReadValue, 0);
			}
			else if (const FTextProperty* TextProperty = CastField<const FTextProperty>(Property))
			{
				FText const& ReadValue = TextProperty->GetPropertyValue(ThisPtr);
				return ReadValue.ToString();
			}
			else if (const FArrayProperty* ArrayProperty = CastField<const FArrayProperty>(Property))
			{
				int32 Index = -1;
				if (FDefaultValueHelper::ParseInt(Indexer, Index))
				{
					if (Index > 0)
					{
						if (ArrayProperty->Inner->IsA<FIntProperty>())
						{
							TArray<int32> IntArray = *ArrayProperty->ContainerPtrToValuePtr<TArray<int32>>(DescribedObject);
							if (IntArray.Num() > Index)
							{
								return FString::FromInt(IntArray[Index]);
							}
						}
					}
				}
				else
				{
					if (ArrayProperty->Inner->IsA<FIntProperty>())
					{
						if (const DescriptionParserArrayCallback* Callback = IndexerMap.Find(Indexer))
						{
							if (Callback->IsBound())
							{
								TArray<int32> IntArray = *ArrayProperty->ContainerPtrToValuePtr<TArray<int32>>(DescribedObject);
								return Callback->Execute(Indexer, IntArray);
							}
						}
					}
				}
			}
			else
			{
				if (Indexer.Len() == 0)
				{
					return FString::Printf(TEXT("<UNSUPPORTED TYPE '%s': %s>"), *(Property->GetCPPType()), *Keyword);
				}
				else
				{
					return FString::Printf(TEXT("<UNSUPPORTED TYPE '%s': %s[%s]>"), *(Property->GetCPPType()), *Keyword, *Indexer);
				}
			}
		}
	}

	if (Indexer.Len() == 0)
	{
		return FString::Printf(TEXT("<INVALID KEYWORD: %s>"), *Keyword);
	}
	else
	{
		return FString::Printf(TEXT("<INVALID KEYWORD: %s[%s]>"), *Keyword, *Indexer);
	}
}

void DescriptionParser::RegisterKeyword(const FString& Keyword, const DescriptionParserCallback& Callback)
{
	KeywordMap.Add(Keyword, Callback);
}

void DescriptionParser::UnregisterKeyword(const FString& Keyword)
{
	KeywordMap.Remove(Keyword);
}

void DescriptionParser::RegisterArrayIndexer(const FString& Indexer, const DescriptionParserArrayCallback& Callback)
{
	IndexerMap.Add(Indexer, Callback);
}

void DescriptionParser::UnregisterArrayIndexer(const FString& Indexer)
{
	IndexerMap.Remove(Indexer);
}
