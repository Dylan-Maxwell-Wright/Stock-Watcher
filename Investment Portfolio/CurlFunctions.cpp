#define _CRT_SECURE_NO_DEPRECATE

#include "pch.h"
#include <curl.h>
#include <easy.h>
#include <string>
#include <iostream>

using std::string;

//Forward Declearations
size_t curlToFile(void *ptr, size_t size, size_t nmemb, FILE* stream);
string convertSymbol(string);


//Update() takes the list of NYSE stocks from the "NYSE_Symbols.txt" file and updates each one using curl and the Alpha Vantage API, API Key = "F7Y9LBJGOVGO7X3Z", Quandl API Key = "s4BrXUWMYNwvU7oRErGy"
//Quandl API for full table of updated (9:15 p.m. pst every weekday) https://www.quandl.com/api/v3/databases/WIKI/metadata?api_key=s4BrXUWMYNwvU7oRErGy - 300 calls every 10 seconds
//https://www.quandl.com/api/v3/datasets/EOD/MSFT.csv?api_key=s4BrXUWMYNwvU7oRErGy

//Alpha Vantage has too low of call rates, but uses a similar system for updating stocks
//Alpha Vantage URL https://www.alphavantage.co/query?function=TIME_SERIES_DAILY_ADJUSTED&symbol=GOOG&apikey=F7Y9LBJGOVGO7X3Z&datatype=csv

//https://www.quandl.com/api/v3/datasets/WIKI/GOOG.csv?rows=1&column_index=11&api_key=s4BrXUWMYNwvU7oRErGy

string convertSymbol(string symbol) //Quandl requires that stock symbols with . and - symbols instead use _ for the url
{
	string newString;
	for (int counter = 0; symbol[counter] != '\0'; counter++)
	{
		if (symbol[counter] == '-' || symbol[counter] == '.')
		{
			symbol[counter] = '_';
		}
		newString += symbol[counter];
	}
	return newString;
}

void singleUpdate(string symbol) //Updates latest price of selected stock to Market_Quote.txt. Discard the first line, and the second line has format "date,adj.price"
{
	string tempSymbol;
	CURLcode returnCode;
	string url;
	FILE* fPointer;

	CURL* oneStockReader = curl_easy_init();
	
	tempSymbol = convertSymbol(symbol); //must pass converted symbol to url, and to WIKI file

	fPointer = fopen("Market_Quote.txt", "w");

	url = "https://www.quandl.com/api/v3/datasets/WIKI/" + tempSymbol + ".csv?rows=1&column_index=11&api_key=s4BrXUWMYNwvU7oRErGy";
	
	curl_easy_setopt(oneStockReader, CURLOPT_URL, url.c_str());
	curl_easy_setopt(oneStockReader, CURLOPT_WRITEFUNCTION, curlToFile);
	curl_easy_setopt(oneStockReader, CURLOPT_WRITEDATA, fPointer);
	returnCode = curl_easy_perform(oneStockReader);

	fclose(fPointer);
	curl_easy_cleanup(oneStockReader);
}
