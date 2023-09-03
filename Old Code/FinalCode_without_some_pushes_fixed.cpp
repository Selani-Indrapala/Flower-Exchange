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

int CheckValidity(string orderID, string ClOrdID, string Instrument, int side, int qty, int price, ofstream& outputFile,const string& timestamp) {
    int flag = 0;

    switch (flag) {
        case 0: {
            string validInstruments[] = { "Rose","Lavender","Lotus","Tulip","Orchid" };
            if (find(begin(validInstruments), end(validInstruments), Instrument) == end(validInstruments)) {
                outputFile << orderID << "," << ClOrdID << "," << Instrument
                    << "," << side << ","<<1<<"," << qty
                    << "," << price << "," << "Invalid Instrument" << "," << timestamp << "\n";
                flag = 1;
                break;
            }
        }
        case 1: {
            int validSides[] = { 1, 2 };
            if (find(begin(validSides), end(validSides), side) == end(validSides)) {
                outputFile << orderID << "," << ClOrdID << "," << Instrument
                    << "," << side <<","<<1<<"," << qty
                    << "," << price << "," << "Invalid Side" << "," << timestamp << "\n";
                flag = 1;
                break;
            }
        }
        case 2: {
            if (price <= 0) {
                outputFile << orderID << "," << ClOrdID << "," << Instrument
                    << "," << side <<","<<1<<"," << qty
                    << "," << price << "," << "Invalid Price" << "," << timestamp << "\n";
                flag = 1;
                break;
            }
        }
        case 3: {
            int rem = qty % 10;
            if (rem != 0 || (qty > 1000) || (qty < 10)) {
                outputFile << orderID << "," << ClOrdID << "," << Instrument
                    << "," << side << ","<<1<<","  << qty
                    << "," << price << "," << "Invalid Size" << "," << timestamp << "\n";
                flag = 1;
                break;
            }
        }
    }

    return flag;
}


class OrderTable {
public:
    OrderTable() {}

    void insertRow(const string& orderID, const string& clientOrder, const string& instrument,
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

        OrderRow temp;

        if (side == 1) {
            int count = 0;
            if (sellTable_.empty()) {
                buyTable_.push(row);
                row.time = timestamp;
                writetoFile(outputFile, row);
                row.qty = 0;
            }
            row.execStatus = 4;
            
            // Check if there are matching sellTable orders
            while (!sellTable_.empty()) {
                OrderRow sellRow = sellTable_.top();

                    if (row.price >= sellRow.price) {
                        count += 1;
                        if (row.qty == sellRow.qty) {
                            // Update the corresponding buyTable order
                            row.execStatus = 2;
                            row.price = sellRow.price;
                            row.time = timestamp;
                            writetoFile(outputFile, row);

                            // Update sellTable
                            sellRow.execStatus = 2;
                            sellRow.time = timestamp;
                            writetoFile(outputFile, sellRow);

                            sellTable_.pop(); // Remove the matching sellTable order
                            row.qty = 0;
                            break;
                        }
                        else if (row.qty > sellRow.qty) {
                            // Update the corresponding buyTable order
                            // Add the PFill row
                            row.execStatus = 3;
                            int remainder = row.qty - sellRow.qty;
                            int init_price = row.price;
                            row.qty = sellRow.qty;
                            row.price = sellRow.price;
                            row.time = timestamp;
                            writetoFile(outputFile, row);

                            // Add the remainder row
                            temp = row;
                             // Remove the matching buyTable order
                            temp.qty = remainder;
                            temp.price = init_price;
                            temp.execStatus = 4;
                            row = temp;

                            // Update sellTable
                            sellRow.execStatus = 2;
                            sellRow.price = row.price;
                            sellRow.time = timestamp;
                            writetoFile(outputFile, sellRow);
                            sellTable_.pop();
                        }
                        else {
                            // Update the corresponding buyTable order
                            row.execStatus = 2;
                            row.time = timestamp;
                            writetoFile(outputFile, row);

                            // Update sellTable
                            int remainder = sellRow.qty - row.qty;
                            int init_price = sellRow.price;
                            sellRow.execStatus = 3;
                            sellRow.qty = row.qty;
                            sellRow.price = row.price;
                            sellRow.time = timestamp;
                            writetoFile(outputFile, sellRow);
                            sellRow.execStatus = 4;
                            sellRow.qty = remainder;
                            row.qty = 0;
                            break;
                        }
                    }
                    else if (row.price < sellRow.price && count == 0) {
                        row.execStatus = 0;
                        row.time = timestamp;
                        buyTable_.push(row);
                        writetoFile(outputFile, row);
                        row.execStatus = 4;
                        row.qty = 0;
                        break;
                    }
                    else {
                        buyTable_.push(row);
                        row.qty = 0;
                        break; // No more matching sellTable orders
                    }
                
            }
            if (row.qty>0){
                buyTable_.push(row);
            }
        }
        else if (side == 2) {
            int count = 0;
            if (buyTable_.empty()) {
                sellTable_.push(row);
                row.time = timestamp;
                writetoFile(outputFile, row);
                row.qty = 0;
            }
            row.execStatus = 4;
            
            // Check if there are matching buyTable orders
            while (!buyTable_.empty()) {
                OrderRow buyRow = buyTable_.top();
                
                    if (row.price <= buyRow.price) {
                        count += 1;
                        if (row.qty == buyRow.qty) {
                            // Update the corresponding sellTable order
                            row.execStatus = 2;
                            row.time = timestamp;
                            writetoFile(outputFile, row);

                            // Update buyTable
                            buyRow.execStatus = 2;
                            buyRow.price = row.price;
                            buyRow.time = timestamp;
                            writetoFile(outputFile, buyRow);

                            buyTable_.pop(); // Remove the matching buyTable order
                            row.qty = 0;
                            break;
                        }
                        else if (row.qty > buyRow.qty) {
                            // Update the corresponding sellTable order
                            // Add the PFill row
                            int remainder = row.qty - buyRow.qty;
                            int init_price = row.price;
                            row.execStatus = 3;
                            row.qty = buyRow.qty;
                            row.price = buyRow.price;
                            row.time = timestamp;
                            writetoFile(outputFile, row);

                            // Add the remainder row
                            temp = row;
                             // Remove the matching sellTable order
                            temp.qty = remainder;
                            temp.price = init_price;
                            temp.execStatus = 4;
                            row = temp;

                            // Update buyTable
                            buyRow.execStatus = 2;
                            buyRow.time = timestamp;
                            writetoFile(outputFile, buyRow);
                            buyTable_.pop();
                        }
                        else {
                            // Update the corresponding sellTable order
                            row.execStatus = 2;
                            row.time = timestamp;
                            writetoFile(outputFile, row);

                            // Update buyTable
                            int remainder = buyRow.qty - row.qty;
                            buyRow.execStatus = 3;
                            buyRow.qty = row.qty;
                            buyRow.time = timestamp;
                            writetoFile(outputFile, buyRow);
                            buyRow.execStatus = 4;
                            buyRow.qty = remainder;
                            row.qty = 0;
                            break;
                        }
                    }
                    else if (row.price < buyRow.price && count == 0) {
                        row.execStatus = 0;
                        row.time = timestamp;
                        sellTable_.push(row);
                        writetoFile(outputFile, row);
                        row.execStatus = 4;
                        row.qty = 0;
                        break;
                    }
                    else {
                        sellTable_.push(row);
                        row.qty = 0;
                        break; // No more matching buyTable orders
                    }
                
            }
            if (row.qty>0){
                sellTable_.push(row);
            }
        }
    }

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
            << "," << fixed << setprecision(2) << row.price << "," << row.reason << "," << row.time << "\n";
    }

    priority_queue<OrderRow, vector<OrderRow>, less<OrderRow>> buyTable_;
    priority_queue<OrderRow, vector<OrderRow>, greater<OrderRow>> sellTable_;
};

int main() {
    ofstream MyFile("output.csv"); // File for output
    MyFile << "Order ID,Cl. Ord. ID,Instrument,Side,Exec Status,Quantity,Price,Reason,Execution Time" << endl; // Output file heading

    ifstream file; // File to be read
    file.open("order7.csv");
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

    // Reading an entire row
    getline(file, line);
    while (getline(file, line)) {
        words = getWords(line); // words is a vector containing the string of each attribute in a row
        Ord_cnt += 1;

        // Items of the order as per the Flower class
        string OrderID = "ord" + to_string(Ord_cnt);
        string Instrument = words[1];

        // Get the current timestamp
        string timestamp = getCurrentDateTime(timestampStream);

        // Check if the order is Valid
        int flag = CheckValidity(OrderID, words[0], Instrument, stoi(words[2]), stoi(words[3]), stoi(words[4]), MyFile,timestamp);
        if (flag == 1) {
            continue;
        }

        // Use the appropriate OrderTable instance based on the instrument
        if (orderTables.find(Instrument) != orderTables.end()) {
            orderTables[Instrument].insertRow(OrderID, words[0], Instrument, stoi(words[2]), stoi(words[3]), stoi(words[4]), MyFile,timestamp);
        }
    }

    MyFile.close();
    file.close();
    return 0;
}
