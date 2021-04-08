// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "sqlite/Public/sqlite3.h"

#include "UOperationLibrary.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FAllDataInformation
{
	GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Weather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Temperature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString TempMAX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString TempMIN;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Humidity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString CloudsSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString DataRequest;
};

USTRUCT(BlueprintType)
struct FCityCount
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Count;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FString> CityName;
};

USTRUCT(BlueprintType)
struct FGetingDataFromSQL
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString DataRequest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString DateTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString DataSQL;
};

UCLASS()
class TESTFROMXSOLLA_API UUOperationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()		
	
public: 
	UFUNCTION(BlueprintCallable)
		static FAllDataInformation ParsingDataFromURLRequest(const FString& Request, const FString& DataTime, FString& OutError);

	UFUNCTION(BlueprintCallable)
		static FString GetDataFromOpenWeatherMap(const FString& InCityName, FString& OutError);

	UFUNCTION(BlueprintCallable)
		static void SaveDataFromSQL(const FString& InCityName, const FString& DataTime, const FAllDataInformation& DataStruct, FString& OutError);

	UFUNCTION(BlueprintCallable)
		static FAllDataInformation GetDataFromSQL(const FString& InCityName, FString& OutError);

	UFUNCTION(BlueprintCallable)
		static FCityCount GetCountCityFromSQL(FString& OutError);

	UFUNCTION(BlueprintCallable)
		static void AddCityFromSQL(const FString& InCityName, FString& OutError);

	static int readBlob(
		sqlite3* db, /* Database containing blobs table */
		const char* zKey, /* Null-terminated key to retrieve blob for */
		unsigned char** pzBlob, /* Set *pzBlob to point to the retrieved blob */
		int* pnBlob, /* Set *pnBlob to the size of the retrieved blob */
		FString& SQL, /* SQL request*/
		int* Count
	);
};
