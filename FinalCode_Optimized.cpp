/**
	FinalCode_Optimized.cpp
	Purpose: Processes an order file and produces the execution report.
	
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <typeinfo>
#include <chrono>
#include <map>
#include <ctime>
#include <iomanip>
#include <queue>
#include <sstream>

using namespace std;

/*
	Takes a line of words separated by commas as input.
	Returns the set of words as a vector.
*/
vector<string> getWords(string s) {
    vector<string> res;
    res.reserve(5);
    string word;
    stringstream ss(s);

    while (getline(ss, word, ',')) {
        res.push_back(word);
    }

    return res;
}

/*
	Returns the current time
*/
string getCurrentDateTime(stringstream& timestampStream) {
    auto now = chrono::system_clock::now();
    auto milliseconds = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;

    time_t now_c = chrono::system_clock::to_time_t(now);
    struct tm parts;
    localtime_s(&parts, &now_c);

    char result[100];
    strftime(result, sizeof(result), "%Y%m%d-%H%M%S", &parts);

    char millisecondStr[4];
    snprintf(millisecondStr, sizeof(millisecondStr), ".%03lld", static_cast<long long>(milliseconds.count()));

    return string(result) + millisecondStr;
}

/*
	Checks the validity of an Order.
    Records the order in the ouput file in the case of being invalid.
*/
int CheckValidity(string orderID, string ClOrdID, string Instrument, int side, int qty, int price, ofstream& outputFile,const string& timestamp) {
    int flag = 0;

    switch (flag) {
        case 0: { //invalid instrument
            string validInstruments[] = { "Rose","Lavender","Lotus","Tulip","Orchid" };
            if (find(begin(validInstruments), end(validInstruments), Instrument) == end(validInstruments)) {
                outputFile << orderID << "," << ClOrdID << "," << Instrument
                    << "," << side << ","<<1<<"," << qty
                    << "," <<fixed<<setprecision(2)<< price << "," << "Invalid Instrument" << "," << timestamp << "\n";
                flag = 1;
                break;
            }
        }
        case 1: { //invalid Side
            int validSides[] = { 1, 2 };
            if (find(begin(validSides), end(validSides), side) == end(validSides)) {
                outputFile << orderID << "," << ClOrdID << "," << Instrument
                    << "," << side <<","<<1<<"," << qty
                    << "," <<fixed<<setprecision(2)<< price << "," << "Invalid Side" << "," << timestamp << "\n";
                flag = 1;
                break;
            }
        }
        case 2: { //invalid price
            if (price <= 0) {
                outputFile << orderID << "," << ClOrdID << "," << Instrument
                    << "," << side <<","<<1<<"," << qty
                    << "," <<fixed<<setprecision(2)<< price << "," << "Invalid Price" << "," << timestamp << "\n";
                flag = 1;
                break;
            }
        }
        case 3: { //invalid Quantity
            int rem = qty % 10;
            if (rem != 0 || (qty > 1000) || (qty < 10)) {
                outputFile << orderID << "," << ClOrdID << "," << Instrument
                    << "," << side << ","<<1<<","  << qty
                    << "," <<fixed<<setprecision(2)<< price << "," << "Invalid Quantity" << "," << timestamp << "\n";
                flag = 1;
                break;
            }
        }
    }

    return flag;
}

/**
	OrderTable
    Keeps a Buy Side and Sell Side to record incoming orders
    that do not find a suitable match.
    One should be created for each instrument.
*/
class OrderTable {
public:
    OrderTable() {}
    /**
	Insert Row method to process a given set of Order attributes,
    Write them to the execution report,
    and record as appropriate
    */
    int insertRow(const string& orderID, const string& clientOrder, const string& instrument,
        int side, int qty, int price, ofstream& outputFile,const string& timestamp) {
        OrderRow row;
        row.orderID = orderID;
        row.clientOrder = clientOrder;
        row.side = side;
        row.execStatus = 0;
        row.qty = qty;
        row.price = price;
        row.reason = " ";
        row.instrument = instrument;

        //Buy side
        if (side == 1) {
            int count = 0;
            if (sellTable_.empty()) { //if sell table is empty
                buyTable_.push(row);
                row.time = timestamp;
                writetoFile(outputFile, row);
                return 1;
            }
            row.execStatus = 4;
            // Check if there are matching sellTable orders
            while (!sellTable_.empty()) {
                    OrderRow sellRow = sellTable_.top();
                    if (row.price >= sellRow.price) {
                        count += 1;
                        if (row.qty == sellRow.qty) {
                            // Write Buy Execution
                            row.execStatus = 2;
                            row.price = sellRow.price;
                            row.time = timestamp;
                            writetoFile(outputFile, row);
                            

                            // Write Sell execution and remove from table
                            sellRow.execStatus = 2;
                            sellRow.time = timestamp;
                            writetoFile(outputFile, sellRow);
                            sellTable_.pop(); // Remove the matching sellTable order
                            return 1;
                        }
                        else if (row.qty > sellRow.qty) {
                            //Write Buy Execution
                            row.execStatus = 3;
                            int remainder = row.qty - sellRow.qty;
                            int init_price = row.price;
                            row.qty = sellRow.qty;
                            row.price = sellRow.price;
                            row.time = timestamp;
                            writetoFile(outputFile, row);

                            //Find the remainder
                            row.qty = remainder;
                            row.price = init_price;
                            row.execStatus = 4;

                            // Write Sell Execution and remove from table
                            sellRow.execStatus = 2;
                            sellRow.price = row.price;
                            sellRow.time = timestamp;
                            writetoFile(outputFile, sellRow);
                            sellTable_.pop();
                        }
                        else {
                            //Write buy execution 
                            row.execStatus = 2;
                            row.time = timestamp;
                            writetoFile(outputFile, row);

                            //Write Sell execution and update sell row
                            int remainder = sellRow.qty - row.qty;
                            int init_price = sellRow.price;
                            sellRow.execStatus = 3;
                            sellRow.qty = row.qty;
                            sellRow.price = row.price;
                            sellRow.time = timestamp;
                            writetoFile(outputFile, sellRow);
                            sellRow.execStatus = 4;
                            sellRow.qty = remainder;//updating sell row
                            return 1;
                        }
                    }
                    else if (row.price < sellRow.price && count == 0) {//New order with no matching pair
                        row.execStatus = 0;
                        row.time = timestamp;
                        buyTable_.push(row);
                        writetoFile(outputFile, row);
                        row.execStatus = 4;
                        return 1;
                    }
                    else {
                        buyTable_.push(row);
                        return 1; // No more matching sellTable orders
                    }
                
            }
            //insert remaining quantity to buy table
            buyTable_.push(row);
            return 1;
        }

        //Sell side
        else if (side == 2) {
            int count = 0;
            if (buyTable_.empty()) {//if buy table is empty
                sellTable_.push(row);
                row.time = timestamp;
                writetoFile(outputFile, row);
                return 1;
            }
            row.execStatus = 4;
            
            // Check if there are matching buyTable orders
            while (!buyTable_.empty()) {
                    OrderRow buyRow = buyTable_.top();               
                    if (row.price <= buyRow.price) {
                        count += 1;
                        if (row.qty == buyRow.qty) {
                            // Write Sell Execution and remove from table
                            row.execStatus = 2;
                            row.time = timestamp;
                            writetoFile(outputFile, row);

                            //Write buy exection
                            buyRow.execStatus = 2;
                            buyRow.price = row.price;
                            buyRow.time = timestamp;
                            writetoFile(outputFile, buyRow);

                            buyTable_.pop(); // Remove the matching buyTable order
                            return 1;
                        }
                        else if (row.qty > buyRow.qty) {
                            //write sell execution
                            int remainder = row.qty - buyRow.qty;
                            int init_price = row.price;
                            row.execStatus = 3;
                            row.qty = buyRow.qty;
                            row.price = buyRow.price;
                            row.time = timestamp;
                            writetoFile(outputFile, row);

                            //Find the remainder
                            row.qty = remainder;
                            row.price = init_price;
                            row.execStatus = 4;

                            // Write Buy Execution and remove from table
                            buyRow.execStatus = 2;
                            buyRow.time = timestamp;
                            writetoFile(outputFile, buyRow);
                            buyTable_.pop();
                        }
                        else {
                            //Write sell execution 
                            row.execStatus = 2;
                            row.time = timestamp;
                            writetoFile(outputFile, row);

                            //Write buy execution and update sell row
                            int remainder = buyRow.qty - row.qty;
                            buyRow.execStatus = 3;
                            buyRow.qty = row.qty;
                            buyRow.time = timestamp;
                            writetoFile(outputFile, buyRow);
                            buyRow.execStatus = 4;
                            buyRow.qty = remainder;//update buy row
                            return 1;
                        }
                    }
                    else if (row.price < buyRow.price && count == 0) {//New order with no matching pair
                        row.execStatus = 0;
                        row.time = timestamp;
                        sellTable_.push(row);
                        writetoFile(outputFile, row);
                        row.execStatus = 4;
                        return 1;
                    }
                    else {
                        sellTable_.push(row);
                        return 1; // No more matching buyTable orders
                    }
                
            }
            //insert remaining quantity to sell table
            sellTable_.push(row);
            return 1;
        }
        return 1;
    }
/*
OrderRow - A structure to hold the attributes of an order
Buy Table, Sell Table - Holds Buy Rows and Sell Rows in a priority queue.
Buy table is descending
Sell table is ascending
*/
private:
    struct OrderRow {
        string orderID;
        string clientOrder;
        string instrument;
        int side;
        int execStatus;
        int qty;
        double price;
        string time;
        string reason;
    

    bool operator>(const OrderRow& other) const {
        return price > other.price;
    }

    bool operator<(const OrderRow& other) const {
        return price < other.price;
    }

    };

    void writetoFile(ofstream& outputFile, const OrderRow& row) {
        outputFile << row.orderID << "," << row.clientOrder << "," << row.instrument
            << "," << row.side << "," << row.execStatus << "," << row.qty
            << "," <<fixed<<setprecision(2)<< row.price << "," << row.reason << "," << row.time << "\n";
    }

    priority_queue<OrderRow, vector<OrderRow>, less<OrderRow>> buyTable_;
    priority_queue<OrderRow, vector<OrderRow>, greater<OrderRow>> sellTable_;
};

int main() {
    ofstream MyFile("execution_rep.csv"); // File for output
    MyFile << "Order ID,Cl. Ord. ID,Instrument,Side,Exec Status,Quantity,Price,Reason,Execution Time" << endl; // Output file heading

    ifstream file; // File to be read

    file.open("orders.csv");

    //Checking if the file is present in the folder
    if (!file.is_open()) {
        cerr << "Error: Could not find 'Orders.csv' for reading. Please make sure the relevant csv file is appropriately named and is in the same folder." << endl;
        return 1;
    }

    string line;
    vector<string> words;
    int Ord_cnt = 0;

    // Create a map to hold OrderTable instances for different instruments
    map<string, OrderTable> orderTables;

    // Initialize OrderTable instances for each instrument
    orderTables["Rose"];
    orderTables["Tulip"];
    orderTables["Lavender"];
    orderTables["Lotus"];
    orderTables["Orchid"];

    // Reusable stringstream for timestamp conversion
    stringstream timestampStream;

    //Execution called each time
    int Execute_Row;

    // Reading an entire row
    getline(file, line);
    while (getline(file, line)) {
        words = getWords(line); // words is a vector containing the string of each attribute in a row
        Ord_cnt += 1;

        // Items of the order as per the Flower class
        string OrderID = "ord" + to_string(Ord_cnt);
        string Instrument = words[1];
        string ClOrd = words[0];
        int side = stoi(words[2]);
        int qty = stoi(words[3]);
        double price = stoi(words[4]);

        // Get the current timestamp
        string timestamp = getCurrentDateTime(timestampStream);

        // Check if the order is Valid
        int flag = CheckValidity(OrderID, ClOrd, Instrument, side, qty, price, MyFile,timestamp);
        if (flag == 1) {
            continue;
        }

        // Use the appropriate OrderTable instance based on the instrument
        Execute_Row = orderTables[Instrument].insertRow(OrderID, ClOrd, Instrument, side, qty, price, MyFile,timestamp);
        
    }

    MyFile.close();
    file.close();
    return 0;
}

