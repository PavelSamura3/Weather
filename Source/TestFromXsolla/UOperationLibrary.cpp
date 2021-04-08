// Fill out your copyright notice in the Description page of Project Settings.


#include "UOperationLibrary.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Misc/DefaultValueHelper.h"
#include <winsqlite/winsqlite3.h>
#include "sqlite/Public/sqlite3.h"
#include <chrono>
#include <ctime>

#include <libcurl/7_48_0/include/Win64/VS2015/curl/curl.h>

PRAGMA_DISABLE_OPTIMIZATION

FAllDataInformation UUOperationLibrary::ParsingDataFromURLRequest(const FString& Request, const FString& DataTime, FString& OutError)
{
	FAllDataInformation ADI;
	OutError = TEXT("");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef< TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(Request);
	TSharedPtr<FJsonObject> jsonObj;
	// Deserialize the JSON data

	if (FJsonSerializer::Deserialize(JsonReader, JsonObject) &&
		JsonObject.IsValid())
	{
		jsonObj = JsonObject->GetObjectField("main");

		ADI.Temperature = jsonObj->GetStringField(TEXT("temp"));
		ADI.TempMAX = jsonObj->GetStringField(TEXT("temp_max"));
		ADI.TempMIN = jsonObj->GetStringField(TEXT("temp_min"));
		ADI.Humidity = jsonObj->GetStringField(TEXT("humidity"));

		TArray<TSharedPtr<FJsonValue>> objArray = JsonObject->GetArrayField("weather");

		jsonObj = objArray[0]->AsObject();

		ADI.Description = jsonObj->GetStringField(TEXT("description"));
		ADI.Weather = jsonObj->GetStringField(TEXT("main"));

		jsonObj = JsonObject->GetObjectField("wind");
		ADI.CloudsSpeed = jsonObj->GetStringField(TEXT("speed"));
		ADI.DataRequest = DataTime;
	}
	
	return ADI;
}


#pragma comment(lib, "urlmon.lib")

#include <urlmon.h>
#include <sstream>

FString UUOperationLibrary::GetDataFromOpenWeatherMap(const FString& InCityName, FString& OutError)
{
	IStream* stream;
	FString resultString;
	OutError = TEXT("");
	//Also works with https URL's - unsure about the extent of SSL support though.

	if (!InCityName.IsEmpty())
	{
		std::string Request = "http://api.openweathermap.org/data/2.5/weather?q=" + std::string(TCHAR_TO_UTF8(*InCityName)) + "&APPID=7edede0d6efff7589b38070370cf8a77";
		std::wstring stemp = std::wstring(Request.begin(), Request.end());


		HRESULT URLresult = URLOpenBlockingStream(0, stemp.c_str(), &stream, 0, 0);

		if (URLresult < 0)
		{
			OutError = FString::Printf(TEXT("Is not valid URL Request: %s"), Request.c_str());
			return resultString;
		}
		else
		{
			char buffer[100];
			unsigned long bytesRead;
			std::stringstream StringStreaming;

			stream->Read(buffer, 100, &bytesRead);

			while (bytesRead > 0U)
			{
				StringStreaming.write(buffer, (long long)bytesRead);
				stream->Read(buffer, 100, &bytesRead);
			}
			stream->Release();

			resultString = StringStreaming.str().c_str();

			return resultString;
		}
	}
	else
	{
		OutError = FString::Printf(TEXT("Not valid City name: %s"), *InCityName);
		return resultString;
	}
}



void UUOperationLibrary::SaveDataFromSQL(const FString& InCityName, const FString& DataTime, const FAllDataInformation& DataStruct, FString& OutError)
{
	FString ConfigPath = FPaths::ProjectDir() / TEXT("DB") / TEXT("WeatherInfo.db");

	sqlite3* DataBase = 0; // hanle from object to connection
	char* ErrorCode = 0;

	std::string Request = "http://api.openweathermap.org/data/2.5/weather?q=" + std::string(TCHAR_TO_UTF8(*InCityName)) + "&APPID=7edede0d6efff7589b38070370cf8a77";
	std::wstring stemp = std::wstring(Request.begin(), Request.end());

	std::string SQL = "UPDATE DataInfo SET NotParsingData = \"" + Request + "\", DataRequest = \"" + std::string(TCHAR_TO_UTF8(*DataTime)) + "\" WHERE City = \"" + std::string(TCHAR_TO_UTF8(*InCityName)) + "\"";
	std::string SQL_2 = "UPDATE FullInfo SET Weather = \"" + std::string(TCHAR_TO_UTF8(*DataStruct.Weather)) + "\", Description = \"" + std::string(TCHAR_TO_UTF8(*DataStruct.Description)) + "\", Temperature = \"" + std::string(TCHAR_TO_UTF8(*DataStruct.Temperature)) + "\", TEmpMAX = \"" + std::string(TCHAR_TO_UTF8(*DataStruct.TempMAX))
		+ "\", TempMIN = \"" + std::string(TCHAR_TO_UTF8(*DataStruct.TempMIN)) + "\", Humidity = \"" + std::string(TCHAR_TO_UTF8(*DataStruct.Humidity)) + "\", CloudsSpeed = \"" + std::string(TCHAR_TO_UTF8(*DataStruct.CloudsSpeed)) + "\" WHERE City = \"" + std::string(TCHAR_TO_UTF8(*InCityName)) + "\"";

	// Open connection
	if (sqlite3_open(TCHAR_TO_UTF8(*ConfigPath), &DataBase))
		OutError = FString::Printf(TEXT("Is not valid URL Request: %s"), sqlite3_errmsg(DataBase));

	// Exec SQL Request
	else if (sqlite3_exec(DataBase, SQL.c_str(), 0, 0, &ErrorCode))
	{
		OutError = FString::Printf(TEXT("Is not valid URL Request: %s"), ErrorCode);
		sqlite3_free(ErrorCode);
	}
	else if (sqlite3_exec(DataBase, SQL_2.c_str(), 0, 0, &ErrorCode))
	{
		OutError = FString::Printf(TEXT("Is not valid URL Request: %s"), ErrorCode);
		sqlite3_free(ErrorCode);
	}
	// Close connection
	sqlite3_close(DataBase);
}

FAllDataInformation UUOperationLibrary::GetDataFromSQL(const FString& InCityName, FString& OutError)
{
	OutError = TEXT("");

	FString ConfigPath = FPaths::ProjectDir() / TEXT("DB") / TEXT("WeatherInfo.db");

	IStream* stream = 0;
	FString resultString;
	FAllDataInformation GDFS;

	sqlite3* DataBase = 0; // handle from object to connection
	char* ErrorCode = 0;

	std::string SQL = "SELECT * FROM FullInfo WHERE City = \"" + std::string(TCHAR_TO_UTF8(*InCityName)) + "\"";
	std::string SQL_2 = "SELECT DataRequest FROM DataInfo WHERE City = \"" + std::string(TCHAR_TO_UTF8(*InCityName)) + "\"";

	// Open connection
	if (sqlite3_open(TCHAR_TO_UTF8(*ConfigPath), &DataBase))
	{
		OutError = FString::Printf(TEXT("Is not valid URL Request: %s"), sqlite3_errmsg(DataBase));		
		return GDFS;
	}
	else
	{
		sqlite3_stmt* stmt = 0;

		if (sqlite3_open(TCHAR_TO_UTF8(*ConfigPath), &DataBase) == SQLITE_OK)
		{
			FString zSQL = SQL_2.c_str();

			int nBlob = 0, Count = 0;
			unsigned char* mBlob;

			readBlob(DataBase, "1", &mBlob, &nBlob, zSQL, &Count);
			GDFS.DataRequest = std::string((char*)mBlob).c_str();

			sqlite3_prepare(DataBase, SQL.c_str(), -1, &stmt, NULL);//preparing the statement
			sqlite3_step(stmt);//executing the statement

			while (sqlite3_column_text(stmt, 0))
			{
				GDFS.Weather = std::string((char*)sqlite3_column_text(stmt, 2)).c_str();
				GDFS.Description = std::string((char*)sqlite3_column_text(stmt, 3)).c_str();
				GDFS.Temperature = std::string((char*)sqlite3_column_text(stmt, 4)).c_str();
				GDFS.TempMAX = std::string((char*)sqlite3_column_text(stmt, 5)).c_str();
				GDFS.TempMIN = std::string((char*)sqlite3_column_text(stmt, 6)).c_str();
				GDFS.Humidity = std::string((char*)sqlite3_column_text(stmt, 7)).c_str();
				GDFS.CloudsSpeed = std::string((char*)sqlite3_column_text(stmt, 8)).c_str();
				
				sqlite3_step(stmt);
			}			
		}

		sqlite3_finalize(stmt);
		sqlite3_close(DataBase);

			return GDFS;
		}

	sqlite3_close(DataBase);
}

FCityCount UUOperationLibrary::GetCountCityFromSQL(FString& OutError)
{
	FCityCount CC;

	std::string SQL = "SELECT City FROM DataInfo";
	int Count = 0;

	FString ConfigPath = FPaths::ProjectDir() / TEXT("DB") / TEXT("WeatherInfo.db");

	FString resultString;

	sqlite3* db;
	sqlite3_stmt* stmt = 0;

	if (sqlite3_open(TCHAR_TO_UTF8(*ConfigPath), &db) == SQLITE_OK)
	{
		sqlite3_prepare(db, SQL.c_str(), -1, &stmt, NULL);//preparing the statement
		sqlite3_step(stmt);//executing the statement

		while (sqlite3_column_text(stmt, 0))
		{
			Count++;
			FString ConvertString(std::string((char*)sqlite3_column_text(stmt, 0)).c_str());
			CC.CityName.Add(ConvertString);
			sqlite3_step(stmt);		
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	CC.Count = Count;

	return CC;
}


void UUOperationLibrary::AddCityFromSQL(const FString& InCityName, FString& OutError)
{
	OutError = TEXT("");
	FString ConfigPath = FPaths::ProjectDir() / TEXT("DB") / TEXT("WeatherInfo.db");
	std::string SQL = "insert into DataInfo(City) values(\"" + std::string(TCHAR_TO_UTF8(*InCityName)) + "\")";
	std::string SQL_2 = "insert into FullInfo(City) values(\"" + std::string(TCHAR_TO_UTF8(*InCityName)) + "\")";

	sqlite3* DataBase = 0; // handle from object to connection
	char* ErrorCode = 0;

	if (sqlite3_open(TCHAR_TO_UTF8(*ConfigPath), &DataBase))
		OutError = FString::Printf(TEXT("Is not valid adding City from DB: %s"), sqlite3_errmsg(DataBase));

	// Exec SQL Request
	else if (sqlite3_exec(DataBase, SQL.c_str(), 0, 0, &ErrorCode))
	{
		OutError = FString::Printf(TEXT("Is not valid adding City from DB: %s"), ErrorCode);
		sqlite3_free(ErrorCode);
	}
	else if (sqlite3_exec(DataBase, SQL_2.c_str(), 0, 0, &ErrorCode))
	{
		OutError = FString::Printf(TEXT("Is not valid adding City from DB: %s"), ErrorCode);
		sqlite3_free(ErrorCode);
	}
	// Close connection
	sqlite3_close(DataBase);
}

int UUOperationLibrary::readBlob(sqlite3* db, const char* zKey, unsigned char** pzBlob, int* pnBlob, FString& SQL, int* Count)
{
	sqlite3_stmt* pStmt;
	int rc;

	/* In case there is no table entry for key zKey or an error occurs,
	 ** set *pzBlob and *pnBlob to 0 now.
	 */
	*pzBlob = 0;
	*pnBlob = 0;
	*Count = 0;

	std::string zSql = std::string(TCHAR_TO_UTF8(*SQL));

	do {
		/* Compile the SELECT statement into a virtual machine. */
		rc = sqlite3_prepare(db, zSql.c_str(), -1, &pStmt, 0);
		if (rc != SQLITE_OK) {
			return rc;
		}

		/* Bind the key to the SQL variable. */
		sqlite3_bind_text(pStmt, 1, zKey, -1, SQLITE_STATIC);

		/* Run the virtual machine. We can tell by the SQL statement that
		 ** at most 1 row will be returned. So call sqlite3_step() once
		 ** only. Normally, we would keep calling sqlite3_step until it
		 ** returned something other than SQLITE_ROW.
		 */
		rc = sqlite3_step(pStmt);
		if (rc == SQLITE_ROW) {
			/* The pointer returned by sqlite3_column_blob() points to memory
			 ** that is owned by the statement handle (pStmt). It is only good
			 ** until the next call to an sqlite3_XXX() function (e.g. the
			 ** sqlite3_finalize() below) that involves the statement handle.
			 ** So we need to make a copy of the blob into memory obtained from
			 ** malloc() to return to the caller.
			 */
			*pnBlob = sqlite3_column_bytes(pStmt, 0);
			*pzBlob = (unsigned char*)malloc(*pnBlob);
			memcpy(*pzBlob, (void*)sqlite3_column_blob(pStmt, 0), *pnBlob);
			*Count = *Count + 1;
		}

		/* Finalize the statement (this releases resources allocated by
		 ** sqlite3_prepare() ).
		 */
		rc = sqlite3_finalize(pStmt);

		/* If sqlite3_finalize() returned SQLITE_SCHEMA, then try to execute
		 ** the statement all over again.
		 */
	} while (rc == SQLITE_SCHEMA);

	return rc;
}
