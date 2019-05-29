#include "pch.h"
#include <iostream>
#include <curl.h>
#include <easy.h>
#include <fstream>
#include <string>
#include <sstream>


using std::string;
using std::cin;
using std::cout;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::endl;

//Forward Declerations
class Portfolio;
void singleUpdate(string);
string convertSymbol(string);


//Functions which read from csv files, combine the data, and create a new text document to hold up-to-date stock market data

string inputToLower(string enteredString)
{
	string finalString, stringReader;
	for (char c : enteredString)
	{
		stringReader = tolower(c);
		finalString.append(stringReader);
	}

	return finalString;
}

size_t curlToFile(void *ptr, size_t size, size_t nmemb, FILE* stream) //Write function for the curl handle which writes the downloaded data to the passed file
{
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

void parseSymbols() //Call this function to create a text file with only the stock symbols on every line, named NYSE_Symbols.txt
{
	string fileReader;
	ifstream stockFile;
	stockFile.open("WIKI_CSV.txt");
	ofstream symbolFile;
	symbolFile.open("WIKI_Symbols.txt");

	getline(stockFile, fileReader); //ignore first line in symbols file

	while (getline(stockFile, fileReader))
	{
		for (int i = 0; fileReader[i] != ','; ++i)
		{
			symbolFile.put(fileReader[i]);
		}
		symbolFile.put('\n');
	}
	stockFile.close();
	symbolFile.close();
}

float priceReader()
{
	ifstream pricesFile;
	float price;
	pricesFile.open("Market_Quote.txt");
	string priceLine;
	std::getline(pricesFile, priceLine, '\n'); //throw away first line
	std::getline(pricesFile, priceLine, ','); //throw away date
	std::getline(pricesFile, priceLine, ','); //get price
	pricesFile.close();
	try
	{
		price = std::stof(priceLine);
		return price;
	}
	catch (std::invalid_argument)
	{
		return 0;
	}
	
	
}

string companyName(string stockSymbol) //check database for company using symbol, and return company's name, or "error" if company is not in database
{
	string compName, revisedName, lineReader, csvReader;
	int nameLength;
	ifstream companyReader;
	companyReader.open("WIKI_CSV.txt");

	std::getline(companyReader, lineReader, '\n'); //discard first line from file
	while (std::getline(companyReader, lineReader, '\n'))
	{
		
		std::stringstream csv(lineReader); //cast to sstream for reading with getline
		std::getline(csv, csvReader, ',');
		string testSymbol = convertSymbol(stockSymbol); //WIKI symbols are already in Quand format, so when reading from any WIKI file, convertSymbol() must be called
		if (csvReader == testSymbol) //If symbol matches
		{
			std::getline(csv, compName, '\n'); //then get name, and return
			companyReader.close();
			nameLength = sizeof(compName);
			revisedName = compName.substr(1, compName.size() - 46); //remove first and last quote from name, 44 characters include "Prices, Dividends, Splits and Trading Volume", which are removed for clarity
			return revisedName;
		}
	}
	
	companyReader.close();
	return "error";
}


