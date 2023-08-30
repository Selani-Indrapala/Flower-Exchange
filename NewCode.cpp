#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <typeinfo>

using namespace std;

vector<string> getWords(string s) {
    vector<string> res;
    int pos = 0;
    while (pos < s.size()) {
        pos = s.find(",");
        res.push_back(s.substr(0, pos));
        s.erase(0, pos + 1); // Length of delimiter is 1
    }
    return res;
}

class OrderTable {
public:
    OrderTable(int initialCapacity = 10) {
        buyTable_.reserve(initialCapacity);
        sellTable_.reserve(initialCapacity);
    }

    void insertRow(const std::string& orderID, const std::string& clientOrder, const std::string& instrument,
               int side, int qty, int price) {
        OrderRow row;
        row.orderID = orderID;
        row.clientOrder = clientOrder;
        row.instrument = instrument;
        row.side = side;
        row.execStatus = "New";
        row.qty = qty;
        row.price = price;

        if (side == 1) {
            buyTable_.push_back(row);
            // Check if there are matching sellTable orders
            for (auto& sellRow : sellTable_) {
                if (row.price >= sellRow.price) {
                    // Update the corresponding sellTable order
                    for (auto& buyRowToUpdate : buyTable_) {
                        buyRowToUpdate.execStatus = "Fill";
                    }

                    //Update sellTable
                    row.orderID = sellRow.orderID;
                    row.clientOrder = sellRow.clientOrder;
                    row.instrument = sellRow.instrument;
                    row.side = sellRow.side;
                    row.execStatus = "Fill";
                    row.qty = sellRow.qty;
                    row.price = sellRow.price;  
                    sellTable_.push_back(row); 
                    break; // Only update the first matching sellTable order
                }
            }
            sort(buyTable_.begin(), buyTable_.end(), [](const OrderRow& a, const OrderRow& b) {
                return a.price > b.price;
            });
            
        } else if (side == 2) {
            sellTable_.push_back(row);
            // Check if there are matching buyTable orders
            for (auto& buyRow : buyTable_) {
                if (row.price <= buyRow.price) {

                    // Update the corresponding sellTable order
                    for (auto& sellRowToUpdate : sellTable_) {
                        sellRowToUpdate.execStatus = "Fill";
                    }

                    //Update buyTable
                    row.orderID = buyRow.orderID;
                    row.clientOrder = buyRow.clientOrder;
                    row.instrument = buyRow.instrument;
                    row.side = buyRow.side;
                    row.execStatus = "Fill";
                    row.qty = buyRow.qty;
                    row.price = buyRow.price;  
                    sellTable_.push_back(row); 
                    break; // Only update the first matching buyTable order
                }
            }            
            sort(sellTable_.begin(), sellTable_.end(), [](const OrderRow& a, const OrderRow& b) {
                return a.price < b.price;
            });
        }
    }

    void printTables(ofstream& outputFile) const {
        printTable(buyTable_, "Buy Table", outputFile);
        printTable(sellTable_, "Sell Table", outputFile);
    }

private:
    struct OrderRow {
        string orderID;
        string clientOrder;
        string instrument;
        int side;
        string execStatus;
        int qty;
        int price;
    };

    void printTable(const vector<OrderRow>& table, const string& tableName, ofstream& outputFile) const {
        cout << tableName << "\n";
        cout << "Order ID | Client Order | Instrument | Side | Exec Status | Quantity | Price\n";
        outputFile << "Order ID,Cl. Ord. ID,Instrument,Side,Exec Status,Quantity,Price" << endl;
        for (const auto& row : table) {
            cout << row.orderID << " | " << row.clientOrder << " | " << row.instrument
                 << " | " << row.side << " | " << row.execStatus << " | " << row.qty
                 << " | " << row.price << "\n";
            outputFile << row.orderID << "," << row.clientOrder << "," << row.instrument
                       << "," << row.side << "," << row.execStatus << "," << row.qty
                       << "," << row.price << "\n";
        }
    }

    vector<OrderRow> buyTable_;
    vector<OrderRow> sellTable_;
};

int main() {
    ofstream MyFile("output.csv"); // File for output
    MyFile << "Order ID,Cl. Ord. ID,Instrument,Side,Exec Status,Quantity,Price" << endl; // Output file heading

    ifstream file; // File to be read
    file.open("order3.csv");
    string line;
    vector<string> words;
    int count = 0;

    OrderTable orders; // Moved the instance creation outside the loop

    // Reading an entire row
    getline(file, line);
    while (getline(file, line)) {
        words = getWords(line); // words is a vector containing the string of each attribute in a row
        count += 1;

        // Items of the order as per the Flower class
        string OrderID = "ord" + to_string(count);
        orders.insertRow(OrderID, words[0], words[1], stoi(words[2]), stoi(words[3]), stoi(words[4]));
    }

    orders.printTables(MyFile); // Print the tables to the output file
    MyFile.close();

    cout << "\nTables printed in the terminal as well.\n";

    file.close();
    return 0;
}
