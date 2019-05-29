#include "pch.h"
#include <stdio.h>
#include <iostream>
#include <curl.h>
#include <easy.h>
#include <vector>
#include <string>
#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>
#include <utility>
#include <tuple>

using std::string;
using std::cin;
using std::cout;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::endl;
using std::to_string;



string inString, fileReader, tempSymbol; //only pass tempSymbol to convertSymbol for use in the url, since convertSymbol takes arguments by reference, and the .txt files should display the origninal symbols
char lineReader[50];
bool readError = false;
bool exitCommand = false;



//Forward Declarations
string inputToLower(string);
string convertSymbol(string);
string companyName(string); //If company is not listed in WIKI, returns string "error"
void parseSymbols();
void singleUpdate(string);
float priceReader();


class Portfolio
{

private: //Price will represent per share prices of specific stocks, value will represent the total of all shares of a stock over the lifetime of trading that stock, and equity will represent total amongst all stocks and shares held
	string Name, fileName;
	string stockName, stockSymbol;
	std::vector <string> portfolioSymbols;
	std::vector <string> temporaryVector;

	//Most of these variables are for a single stock at a time, besides numberOfCompanies and totalEquity, and will be updated every time a new stock is looked at.
	unsigned int shares, numberOfCompanies;
	float currentPrice, buyPrice;
	double currentValue, buyValue;
	double totalInvested, totalEquity, profit_loss, totalProfit_loss;
	double capitalGains; //This should be the variable to track all money the profile has made by selling shares

	void backgroundLoopUpdater(string symbol) //when looping through symbols vector, this will update the portfolio in the background
	{
		singleUpdate(symbol);
		stockLoader(symbol);
		updatePortfolio();
	}
	void displayLoopUpdater(string symbol) //when asked to display info, looping through this will update and then print the current portfolio stats
	{
		singleUpdate(symbol);
		stockLoader(symbol);
		updatePortfolio();
		printInfo();
	}

public:

	Portfolio();
	Portfolio(string);

	bool stockOwned = false;
	bool cashedOut = false; //Used by update to remoe line if all shares are sold

	string GetName() { return Name; };
	void SetName(string newName) { Name = newName; fileName = Name + ".txt";};
	unsigned int GetShares() { return shares; }
	double GetValue() { return currentValue; }
	string GetFileName() { return fileName; }
	string GetSymbol() { return stockSymbol; }
	string GetStockName() { return stockName;  }
	//float GetCurrentPrice() { return currentPrice; }

	void stockLoader(string symbol) //load an owned stock into live portfolio variables. If unowned, set stockOwned to false. Call before updating price changes
	{
		stockOwned = false;
		string company = companyName(symbol);
		stockSymbol = symbol;

		if (company == "error") //this should never be the case for internal function calls. It can only have an output when the user inputs an untracked company
		{
			stockName = company; //This makes the stock name "error"
			cout << "The Provided symbol does not match a company in our database." << endl;
			cout << "Please try again. " << endl;
		}

		else
		{
			stockName = company;
			ifstream fReader;
			string lineReader, csvReader;
			fReader.open(fileName);

			std::getline(fReader, lineReader); //Throw away first line (general info)

			while (std::getline(fReader, lineReader, '\n'))  //grab each line ||symbol, current price per share, current shares owned, current value, total money in(buy value), current gains/losses||
			{
				std::stringstream csv(lineReader); //cast to sstream for reading with getline
				std::getline(csv, csvReader, ','); //seperate first element (symbol)
				if (csvReader == symbol) //if match, read data
				{
					/*std::getline(csv, csvReader, ',');
					currentPrice = std::stof(csvReader);
					std::getline(csv, csvReader, ',');
					shares = std::stoi(csvReader);
					std::getline(csv, csvReader, ',');
					currentValue = std::stod(csvReader);
					std::getline(csv, csvReader, ',');
					buyValue = std::stod(csvReader);
					std::getline(csv, csvReader, ',');
					profit_loss = std::stod(csvReader);*/
					stockOwned = true;
					//Same as:
					transcribe(lineReader);
					break;
					cout << "Break didn't exit loop" << endl;
				}
			}

			/*
			if (stockOwned == false)  These lines should be part of a display function, only called when the user is viewing his stocks. Stockloader is called whenever the program mentions a stock, including when a user wants to view it
			{
				cout << "Stock not owned." << endl;
			}
			else
			{
				cout << company << ", current price per share: $" << currentPrice << ". Number of shares owned: " << shares << ". Current value of investment: $" << currentValue << ". Total purchase value: $" << buyValue << ". Overall profits/losses: $" << endl;
				stockOwned = false; //reset tracker
				//profits/losses track positive and negative, but currently this will be displayed as $-80 if there is an $80 loss. Need a function to parse the negative for this and for total profit/loss 
			}
			*/
			if (stockOwned == false)
			{
				buyPrice = 0;
				currentValue = 0;
				shares = 0;
				buyValue = 0;
				currentPrice = priceReader();
			}
			else if (stockOwned == true)
			{
				singleUpdate(symbol);
				priceChangeUpdate();
			}
			
			fReader.close();
		}
	}
	//This function doesn't work. If there is a new stock, the whole portfolio needs to be updated because total invested, new total equity, and new companies owned need to be updated
	void newStock(string symbol) //for previously unowned stocks, create a new line of data based on what is currently stored in Portfolio's variables
	{
		fstream fWriter;
		string lineOutput;

		fWriter.open(fileName, std::ios::app);
		lineOutput = symbol + ',' + to_string(currentPrice) + ',' + to_string(shares) + ',' + to_string(currentValue) + ',' + to_string(buyValue) + ',' + to_string(profit_loss) + '\n';
		fWriter.write(lineOutput.c_str(), sizeof(lineOutput.c_str()));
		fWriter.close();
	}
	void transcribe(string stockLine) //Extracts a single stock's information from a string extracted from the file, and stores it into live variables
	{
		string temp;
		unsigned int j = 0;
		for (unsigned int i = 0; i < stockLine.length(); i++)
		{
			char c = stockLine[i];

			if (c != ',') //ignore commas for parsing
			{
				temp += c;
			}

			if (c == ',') //each time a comma is encountered, the string stored in temp is assigned to a variable according to the order inside the switch statement below
			{
				switch (j)
				{
				case 0: //stock symbol, throw away 
					temp.clear();
					break;

				case 1:
					currentPrice = std::stof(temp);
					temp.clear();
					break;

				case 2:
					shares = std::stoi(temp);
					temp.clear();
					break;

				case 3:
					currentValue = std::stod(temp);
					temp.clear();
					break;

				case 4:
					buyValue = std::stod(temp);
					temp.clear();
					break;

				}
				

				j++;
				
			}
		}
		//no last comma, so profit_loss needs to be stored explicitly after loop ends
		profit_loss = std::stod(temp);
		temp.clear();
	} 
	void printInfo() //Display the current live stock's information in the console, call singleUpdate(symbol) and priceChangeUpdate() before using this function for live results
	{
		double tempLoss;
		if (stockOwned)
		{
			if (profit_loss < .01 || profit_loss > -.01)
			{
				profit_loss = 0; //Correct for less than penny fluctuations
			}
			if (profit_loss < 0)
			{
				tempLoss = 0 - profit_loss;
				cout << stockName << ", current price per share: $" << currentPrice << ". Number of shares owned: " << shares << ". Current value of investment: $" << currentValue << ". Total invested: $" << buyValue << ". Overall losses: -$" << tempLoss << endl;
			}
			else
			{
				cout << stockName << ", current price per share: $" << currentPrice << ". Number of shares owned: " << shares << ". Current value of investment: $" << currentValue << ". Total invested: $" << buyValue << ". Overall profits: $" << profit_loss << endl;
			}
		}
		else if (!stockOwned)
		{
			cout << stockName << ", current price per share: $" << currentPrice << ". Stock Not Owned. " << endl;
		}
	}
	void loadFile() //copies all existing portfolio file stock symbols to the vector and updates all general portfolio variables (total equity, total invested (buyValue), total profit_loss, and gains)
	{
		string lineReader, symbolCopy;

		ifstream portfolioFile;
		portfolioFile.open(fileName);
		std::getline(portfolioFile, lineReader, '\n'); //store first line in lineReader (general info)

		string gainsReader;
		std::stringstream capitalGainsFinder(lineReader); //all general info can be found from the sum of individual stocks except the total capital gains info, which is not tracked by the individual stocks
		for (int i = 0; i <= 5; i++)
		{
			std::getline(capitalGainsFinder, gainsReader, ','); //discard 5 pieces of info, grab the 6th
		}
		
		totalInvested = 0;
		totalEquity = 0;
		totalProfit_loss = 0;

		while (std::getline(portfolioFile, lineReader, '\n'))
		{
			string temp = lineReader;
			string valueReader;

			std::stringstream csv(lineReader); //cast line to stringstream for parsing
			std::getline(csv, symbolCopy, ','); //get symbol
			portfolioSymbols.push_back(symbolCopy);
			
			std::getline(csv, valueReader, ','); //current pps
			currentPrice = std::stof(valueReader);

			std::getline(csv, valueReader, ',');//shares
			shares = std::stoi(valueReader);

			std::getline(csv, valueReader, ','); //value
			currentValue = std::stod(valueReader);

			std::getline(csv, valueReader, ','); //buy value
			buyValue = std::stod(valueReader);
			totalInvested += buyValue;

			std::getline(csv, valueReader, ','); //profit
			profit_loss = std::stod(valueReader);

			totalEquity += currentValue;
			totalProfit_loss += profit_loss;

		}

		numberOfCompanies = portfolioSymbols.size();
		if (numberOfCompanies != 0) //if no companies are owned, an exception is thrown here
		{
			capitalGains = std::stod(gainsReader);
		}
		portfolioFile.close();
	}
	void updatePortfolio() //If any changes occured to the currently viewed stock, update the single stock and save in the portfolio
	{
		string originalReader, tempWriter, symbolChecker, newData;

		ifstream original;
		original.open(fileName);

		ofstream tempFile;
		tempFile.open("temporary.txt");
		
		newData = stockSymbol + ',' + std::to_string(currentPrice) + ',' + std::to_string(shares) + ',' + std::to_string(currentValue) + ',' + std::to_string(buyValue) + ',' + std::to_string(profit_loss) + '\n';

		if (stockOwned == false) //if stock unowned, update number of companies before loading that value into the file, and add symbol to vector
		{
			numberOfCompanies += 1;
		}
		else if (cashedOut)
		{
			numberOfCompanies -= 1;
		}

		//Since the file is to be copied, the first line will be thrown away and updated
		std::getline(original, tempWriter, '\n'); //Throw away first line (general info)
		tempWriter = Name.c_str() + ',' + std::to_string(numberOfCompanies) + ',' + std::to_string(totalEquity) + ',' + std::to_string(totalInvested) + ',' + std::to_string(totalProfit_loss) + ',' + std::to_string(capitalGains) + '\n'; //Input updated info
		tempFile.write(tempWriter.c_str(), tempWriter.size());

		newData = stockSymbol + ',' + std::to_string(currentPrice) + ',' + std::to_string(shares) + ',' + std::to_string(currentValue) + ',' + std::to_string(buyValue) + ',' + std::to_string(profit_loss) + '\n';

		while (std::getline(original, originalReader, '\n')) //Copy the original portoflio file up until the loaded stock;
		{
			std::stringstream csv(originalReader); //cast line to stringstream for parsing, to match symbol for line to change
			std::getline(csv, symbolChecker, ',');

			if (symbolChecker == stockSymbol) //if loaded stock is in file
			{
				if (cashedOut)
				{
					//Do nothing, so new data is not written, and a line is effectively deleted				
				}
				else 
				{
					tempFile.write(newData.c_str(), newData.size()); //if on the line of the current stock, write updated data
				}
			}
			else
			{
				tempWriter = originalReader + '\n';
				tempFile.write(tempWriter.c_str(), tempWriter.size()); //else, copy old data, add newline char (lost with getline)
			}
		}

		if (stockOwned == false) //Check if stock was already updated. If unowned, new line must be added. This whole function is always needed to update general info, so append cannot be used
		{
				tempFile.write(newData.c_str(), newData.size());
		}

		

		original.close();
		tempFile.close();
		
		int remove1 = remove(fileName.c_str());
		int rename1 = rename("temporary.txt", fileName.c_str());

	}
	void priceChangeUpdate() //Call after using singleUpdate(symbol) and stockLoader(symbol)
	{
		float newPrice;
		double tempValue, valueChange;
		newPrice = priceReader(); //priceReader() takes the price from Market_Quote.txt and returns it

		if (stockOwned)
		{
			//Update Equity and value changes
			totalEquity -= currentValue;
			tempValue = shares * newPrice;
			valueChange = tempValue - currentValue; //If new tempValue is greater, then value change is positive
			currentValue = tempValue;
			totalEquity += currentValue;

			//Updtate profits and losses
			totalProfit_loss -= profit_loss;
			profit_loss += valueChange;
			totalProfit_loss += profit_loss;
		}

		currentPrice = newPrice;
	}
	void buyShares(unsigned int moreStocks) 
	{
		double moneyIn = moreStocks * currentPrice;
		buyValue += moneyIn;
		totalEquity += moneyIn;
		currentValue += moneyIn;
		shares += moreStocks;
		updatePortfolio();

		cout << moreStocks << " shares have been purchased for a total of $" << moneyIn << ". Your new portfolio for this company is as follows: " << endl;

		if (stockOwned == false)//only false if stock was previously unowned, and is new to the portfolio
		{
			portfolioSymbols.push_back(stockSymbol);
			stockOwned = true;
		}
		printInfo();
	}
	void sellShares(unsigned int lessShares) //only call if number of shares to sell is less than or equal to number of shares owned. If equal to, remove the company from the profile outside this function
	{
		double moneyOut = lessShares * currentPrice;
		double gains;
		totalEquity -= moneyOut;
		currentValue -= moneyOut;
		shares -= lessShares;
		
		if (moneyOut <= buyValue)
		{
			buyValue -= moneyOut;
		}
		else if (moneyOut > buyValue)
		{
			gains = moneyOut - buyValue; //Any money made pays off the money put in
			buyValue = 0; //If cash sold is greater than the investment, investment is paid off
			capitalGains += gains; //excess becomes capital gains for your profile
		}

		if (cashedOut)
		{
			capitalGains -= buyValue; //If cashed out, and less money was made than was initially invested, remove this from capital gains
			buyValue = 0; //For continuity and safety of data, set investment to 0;

			temporaryVector.clear();
			for (int i = 0; i < portfolioSymbols.size(); i++)
			{
				if (stockSymbol != portfolioSymbols[i])
				{
					temporaryVector.push_back(portfolioSymbols[i]);
				}
			}
			portfolioSymbols.clear();
			portfolioSymbols = temporaryVector;
			temporaryVector.clear();
		}
		updatePortfolio();

		cout << lessShares << " shares have been sold for a total of $" << moneyOut << ". Your new portfolio for this company is as follows: " << endl;
		printInfo();

	}
	void investMoney(double moreMoney)
	{
		unsigned int moreShares;
		moreShares = moreMoney / currentPrice;
		double moneyIn;
		moneyIn = moreShares * currentPrice;
		buyValue += moneyIn;
		totalEquity += moneyIn;
		currentValue += moneyIn;
		shares += moreShares;
		updatePortfolio();

		//Not all of the money can be used to buy shares, since you need to buy whole shares, so any small amount of money left over will be tracked and given back to the user. (handle less than one share case in main())
		double refund = moreMoney - moneyIn;
		if (refund != 0)
		{
			cout << moreShares << " shares have been purchesed for a total of $" << moneyIn << ". Since each share costs $" << currentPrice << ", you have a remainder of $" << refund << " which will be refunded to you. Your new portfolio for this company is as follows: " << endl;
		}
		else
		{
			cout << moreShares << " shares have been purchased for a total of $" << moneyIn << ". Your new portfolio for this company is as follows: " << endl;
		}

		if (stockOwned == false) //only false if stock was previously unowned, and is new to the portfolio
		{
			portfolioSymbols.push_back(stockSymbol);
			stockOwned = true;
		}
		printInfo();
	}
	void divestMoney(double lessMoney)
	{
		unsigned int lessShares;
		double gains;
		lessShares = lessMoney / currentPrice;
		double moneyOut;
		moneyOut = lessShares * currentPrice;
		totalEquity -= moneyOut;
		currentValue -= moneyOut;
		shares -= lessShares;

		if (moneyOut <= buyValue)
		{
			buyValue -= moneyOut;
		}
		else if (moneyOut > buyValue)
		{
			gains = moneyOut - buyValue; //Any money made pays off the money put in
			buyValue = 0; //If cash sold is greater than the investment, investment is paid off
			capitalGains += gains; //excess becomes capital gains for your profile
		}

		if (cashedOut)
		{
			capitalGains -= buyValue; //If cashed out, and less money was made than was initially invested, remove this from capital gains
			buyValue = 0; //For continuity and safety of data, set investment to 0;

			temporaryVector.clear();
			for (int i = 0; i < portfolioSymbols.size(); i++)
			{
				if (stockSymbol != portfolioSymbols[i])
				{
					temporaryVector.push_back(portfolioSymbols[i]);
				}
			}
			portfolioSymbols.clear();
			portfolioSymbols = temporaryVector;
			temporaryVector.clear();
		}

		updatePortfolio();
		
		//If money disired does not lead to an even number of shares, the cash paid out will be rounded down, and the user will be told the difference. (handle less than one share case in main())
		double difference = lessMoney - moneyOut;
		if (difference != 0)
		{
			cout << lessShares << " shares have been sold for a total of $" << moneyOut << ". This is $" << difference << " less than was asked for, but only whole shares can be sold, and each share is currently worth $" << currentPrice << "." << endl;
		}
		else
		{
			cout << lessShares << " shares have been sold for a total of $" << moneyOut << "." << endl;
		}

		printInfo();
	}
	void backgroundPortfolioUpdate() //call for price updates to a whole portfolio, called after buy and sell orders
	{
		//use vector, loop through, update price, update portfolio.
		for (int i = 0; i < portfolioSymbols.size(); i++)
		{
			backgroundLoopUpdater(portfolioSymbols[i]);
		}
	}
	void displayPortfolioUpdate() //call when the user asks to see updated portfolio data
	{
		if (numberOfCompanies == 0)
		{
			cout << "You do not currently own shares of any company. Please enter \"buy\" if you would like to buy stocks, \"search\" if you would like to look at company prices, or \"help\" for a list of commands." << endl;
		}
		else 
		{
			//use vector, loop through, update price, update portfolio, and print all info to console
			for (int i = 0; i < portfolioSymbols.size(); i++)
			{
				displayLoopUpdater(portfolioSymbols[i]);
			}
		}
	}
};

Portfolio::Portfolio()
{
	shares = 0;
	numberOfCompanies = 0;
	currentPrice = 0;
	buyPrice = 0;
	currentValue = 0;
	buyValue = 0;
	totalInvested = 0;
	totalEquity = 0;
	profit_loss = 0;
	totalProfit_loss = 0;
	capitalGains = 0;
}
Portfolio::Portfolio(string ownerName)
{
	Name = ownerName;
	fileName = Name + ".txt";
	ifstream file;
	file.open(fileName);
	

	if (file.is_open())
	{
		file.close();
		loadFile(); //this loads to the temporary portfolio, which is deleted after the login step. Need to call load again afterwards.
		cout << "Welcome back " << Name << "! You currently own stocks with " << numberOfCompanies << " companies, which combined give your portfolio a total value of $" << totalEquity << ". Through selling shares, your total capital gains have been $" << capitalGains << '.' << endl;

	}
	else
	{
		cout << "Welcome " << Name << ". Your profile has been created. Now, we will begin selecting stocks to purchase. " << endl; //fstream cannot create files, so this is how I tell if there was already a file or not
		totalEquity = 0;
		numberOfCompanies = 0;
		profit_loss = 0;
		ofstream newFile;
		newFile.open(fileName); 
		newFile.close();
		
	}
}


int main()
{
	bool quitCommand = false;
	bool logOut = false;
	
	Portfolio user;

	parseSymbols(); //use to generate stock symbol file, "NYSE_Symbols.txt" from NYSE coompany symbols and names file, "NYSE_CSV.txt", or "WIKI_CSV.txt" from "WIKI_Symbols.txt". 

	do { //for loging out and switching profiles
		logOut = false;

		do {
			readError = false;
			memset(lineReader, 0, sizeof(lineReader));
			cout << "Please enter your name: ";
			cin.getline(lineReader, sizeof(lineReader));
			
			inString = lineReader;



			if (cin.fail())
			{
				cout << "There was an error, please keep your name below 50 characters." << endl;
				readError = true;
				cin.clear();
				cin.ignore(32767, '\n');
			}

			else if (inString == "")
			{
				cout << "No name provided. ";
				readError = true;
			}

			else
			{
				Portfolio user = Portfolio(lineReader);
			}

		} while (readError);

		user.SetName(lineReader); //Temp object destroyed after constructor is called, so some stats need to be set outside the control structure
		user.loadFile();
		user.backgroundPortfolioUpdate();

		do { //logged in, base commands
			readError = false;
			exitCommand = false;

			cout << "Please enter a command: ";
			cin.getline(lineReader, sizeof(lineReader));
			inString = inputToLower(lineReader);
			


			if (inString == "exit" || inString == "quit")
			{
				quitCommand = true;
				cout << "Goodbye " << user.GetName();
			}

			else if (inString == "help")
			{
				cout << "The commands are as follows :" << endl <<
					"help      -----  List all of the commands" << endl <<
					"buy       -----  Begin the process of buying additional shares of stocks you already own, or of a company you do not yet own" << endl <<
					"sell      -----  Sell shares of stocks you already own at their current market value. Any earned money goes towards paying off invested values, and once paid off, the rest of the proceedswill go towards your profile's capital gains." << endl <<
					"search    -----  Look for a specific company by name or by symbol, and see their current price  per share, how many shares you own, and their total value" << endl <<
					"portfolio -----  Display your current portfolio's data, including all stocks currently owned, current price of each stock, shares owned, total equity in each, "
					"profits/losses of each stock and your whole portfolio, and the capital gains earned from selling stocks." << endl <<
					"log out   -----  Log out of the current profile, save all changes made, and go to the starting command screen." << endl <<
					"quit      -----  Exit the program, save any completed sales or purchases, but discard any changes in progress. Can enter at any time to quit." << endl << endl;
			}

			else if (inString == "buy")
			{
				bool exitCommand1 = false;
				bool failedAttempt = false;

				do {

					if (failedAttempt == false)
					{
						cout << "Please enter the symbol of the stock you would like to purchase (case sensitive): ";
					}
					else if (failedAttempt == true)
					{
						cout << "Please enter the symbol of the stock you would like to purchase (case sensitive), or enter \"back\" if you would like to enter a different command: ";
						failedAttempt = false;
					}

					cin.getline(lineReader, 32767, '\n');
					string companySymbol = lineReader;
					string company = companyName(lineReader);
					string command = inputToLower(lineReader);


					if (command == "back")
					{
						exitCommand1 = true;
					}
					else if (command == "quit")
					{
						quitCommand = true;
						exitCommand1 = true;
					}
					else if (company == "error")
					{
						failedAttempt = true;
						cout << "The entered company is not in our database, or the entered command is invalid. Please try again." << endl;
					}
					else
					{
						user.stockLoader(companySymbol);
						singleUpdate(companySymbol);
						user.priceChangeUpdate(); //This function was moved to stock loader, so it needs to be manually called after singleUpdate
						user.updatePortfolio();
						user.printInfo();
						bool exitCommand2 = false;
						bool failedAttempt1 = false;

						do {
							if (failedAttempt1)
							{
								cout << "Please enter \"Shares\" to purchase a specific number of shares, or \"Value\" to specify the amount of money you wish to invest, or \"back\" if you would like to enter a different command: ";
								failedAttempt1 = false;
							}
							else
							{
								cout << "Please enter \"Shares\" to purchase a specific number of shares, or \"Value\" to specify the amount of money you wish to invest: ";
							}

							cin.getline(lineReader, 32767, '\n');

							command = inputToLower(lineReader);

							if (command == "back")
							{
								exitCommand2 = true;
							}
							else if (command == "quit")
							{
								quitCommand = true;
							}
							else if (command == "shares")
							{
								try
								{
									unsigned int inputShares;								
									cout << "Please enter the number of shares you would like to purchase: ";									
									cin.getline(lineReader, 32767, '\n');
									inputShares = std::stoi(lineReader);
									//handle exception for bad input
									user.buyShares(inputShares);
									exitCommand2 = true;
									exitCommand1 = true;
								}
								catch (std::invalid_argument)
								{
									cout << "Invalid input. Once Shares or Value has been selected, only enter the amount you would like to purchase." << endl;
									failedAttempt1 = true;									
								}
							}
							else if (command == "value")
							{
								try
								{
									double inputMoney;									
									cout << "Please enter the amount of money you would like to invest: ";								
									cin.getline(lineReader, 32767, '\n');
									inputMoney = std::stod(lineReader);
									//handle exception for bad input
									user.investMoney(inputMoney);
									exitCommand2 = true;
									exitCommand1 = true;
								}
								catch (std::invalid_argument)
								{
									cout << "Invalid input. Once Shares or Value has been selected, please only enter the amount you would like to purchase." << endl;
									failedAttempt1 = true;
								}
							}
							else
							{
								cout << "Invalid input, please try again. " << endl;
							}
						} while (!exitCommand2 && quitCommand == false);
					}


				} while (!exitCommand1 && quitCommand == false);
			}

			else if (inString == "sell")
			{

				bool exitCommand1 = false;
				bool failedAttempt = false;

				do {

					if (failedAttempt == false)
					{
						cout << "Please enter the symbol of the stock you would like to sell (case sensitive): ";
					}
					else if (failedAttempt == true)
					{
						cout << "Please enter the symbol of the stock you would like to sell (case sensitive), or enter \"back\" if you would like to enter a different command: ";
						failedAttempt = false;
					}

					cin.getline(lineReader, 32767, '\n');
					string companySymbol = lineReader;
					string command = inputToLower(lineReader);
					string company = companyName(lineReader);



					if (command == "back")
					{
						exitCommand1 = true;
					}
					else if (command == "quit")
					{
						quitCommand = true;
						exitCommand1 = true;
					}
					else if (company == "error")
					{
						failedAttempt = true;
						cout << "The entered company is not in our database, or the entered command is invalid. Please try again." << endl;
					}
					else
					{
						user.stockLoader(companySymbol);
						if (user.stockOwned == false)
						{
							cout << "You do not currently own any shares of this company. Please try another company, or enter \"back\" to enter another command. " << endl;
							failedAttempt = true;
						}
						else
						{
							singleUpdate(companySymbol); //symbol automatically converted in singleUpdate
							user.priceChangeUpdate();
							user.updatePortfolio();
							user.printInfo();
							bool exitCommand2 = false;
							bool oversold = false;
							bool failedAttempt1 = false;

							do {
								if (failedAttempt1)
								{
									cout << "Please enter \"Shares\" to sell a specific number of shares, \"Value\" to specify the amount of money you wish to divest, or \"back\" to enter a different command: ";
									failedAttempt1 = false;
								}
								else
								{
									cout << "Please enter \"Shares\" to sell a specific number of shares, or \"Value\" to specify the amount of money you wish to divest: ";
								}

								cin.getline(lineReader, 32767, '\n');
								command = inputToLower(lineReader);

								if (command == "back")
								{
									exitCommand2 = true;
								}
								else if (command == "quit")
								{
									quitCommand = true;
								}
								else if (command == "shares")
								{
									try
									{
										do {

											failedAttempt = false;
											oversold = false;
											unsigned int inputShares;

											cout << "Please enter the number of shares you would like to sell: ";
											cin.getline(lineReader, 32767, '\n');
											inputShares = std::stoi(lineReader);

											unsigned int ownedShares = user.GetShares();
											if (inputShares > ownedShares)
											{
												oversold = true;
												failedAttempt = true;
												cout << "You do not own enough shares. Please enter a number of shares equal to or less than the ammount owned." << endl;
											}
											else if (inputShares == ownedShares)
											{
												user.cashedOut = true; //communicate to updatePortfolio() inside sellShares() to remove company from the portfolio
												cout << "You have sold all of your shares for this company. ";
												user.sellShares(inputShares);
												user.cashedOut = false;
												exitCommand1 = true;
												exitCommand2 = true;
											}
											else
											{
												//handle exception for bad input
												user.sellShares(inputShares);
												exitCommand2 = true;
												exitCommand1 = true;
											}
										} while (oversold || failedAttempt);
									}
									catch (std::invalid_argument)
									{
										cout << "Invalid input. Once Shares or Value has been selected, only enter the amount you would like to sell." << endl;
										failedAttempt1 = true;
									}
								}
								else if (command == "value")
								{
									try
									{
										do {
											failedAttempt = false;
											oversold = false;
											double inputMoney;
											cout << "Please enter the amount of money you would like to divest: ";
											cin.getline(lineReader, 32767, '\n');
											inputMoney = std::stod(lineReader);
											double ownedValue = user.GetValue();
											//handle exception for bad input
											if (inputMoney > ownedValue)
											{
												oversold = true;
												failedAttempt = true;
												cout << "You do not own enough shares. Please enter a value equal to or less than the money currently invested." << endl;
											}
											else if (inputMoney == ownedValue)
											{
												user.cashedOut = true; //communicate to updatePortfolio() inside sellShares() to remove company from the portfolio
												cout << "You have sold all of your shares for this company. ";
												user.divestMoney(inputMoney);
												user.cashedOut = false;
												exitCommand1 = true;
												exitCommand2 = true;
											}
											else
											{
												//handle exception for bad input
												user.divestMoney(inputMoney);
												exitCommand2 = true;
												exitCommand1 = true;
											}

										} while (oversold || failedAttempt);
									}
									catch (std::invalid_argument)
									{
										cout << "Invalid input. Once Shares or Value has been selected, only enter the amount you would like to sell." << endl;
										failedAttempt1 = true;
									}
								}
								else
								{
									cout << "Invalid input, please try again. " << endl;
								}

							} while (!exitCommand2 && quitCommand == false);
						}
					}



				} while (!exitCommand1 && quitCommand == false);
			}

			else if (inString == "search")
			{
				bool exitCommand1 = false;
				bool failedAttempt = false;
				bool secondCycle = false;

				do {
					if (failedAttempt == false)
					{
						if (secondCycle) //slightly different prompt for looking at more than one stock
						{
							cout << "Please enter the symbol of another stock you would like to view (case sensitive), or enter \"back\" if you would like to enter a different command: ";
						}
						else
						{
							cout << "Please enter the symbol of the stock you would like to view (case sensitive): ";
						}
					}
					else if (failedAttempt == true)
					{
						cout << "Please enter the symbol of the stock you would like to view (case sensitive), or enter \"back\" if you would like to enter a different command: ";
						failedAttempt = false;
					}


					cin.getline(lineReader, 32767, '\n');
					string companySymbol = lineReader;
					string command = inputToLower(lineReader);
					
					string company = companyName(lineReader);

					
					if (command == "back")
					{
						exitCommand1 = true;
					}
					else if (command == "quit")
					{
						quitCommand = true;
						exitCommand1 = true;
					}
					else if (company == "error")
					{
						failedAttempt = true;
						cout << "The entered company is not in our database, or the entered command is invalid. Please try again." << endl;
					}
					else
					{
						singleUpdate(companySymbol);
						user.stockLoader(companySymbol);
						user.priceChangeUpdate();
						user.printInfo();
						secondCycle = true;
					}

				} while (!exitCommand1 && !quitCommand);
			}

			else if (inString == "portfolio")
			{
				user.displayPortfolioUpdate();
			}

			else if (inString == "log out" || inString == "logout")
			{
				user.backgroundPortfolioUpdate();
				string signOff = user.GetName();
				cout << "Profile saved, " << signOff << " logged out." << endl;
				logOut = true;
			} 

			else if (cin.fail())
			{
				cin.clear();
				readError = true;
				cin.ignore(36727, '\n');
				cout << "Error. Please keep the command below 50 characters. " << endl;
			}
			else
			{
				readError = true;
				cout << "Invalid Command. Type \"help\" for a list of valid commands." << endl;
			}

		} while ((readError || !exitCommand) && (quitCommand == false && logOut == false));

	} while (quitCommand == false || logOut == true);
}

