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

using namespace std;

// Define a constant for the batch size
const int BATCH_SIZE = 1000; // Adjust the batch size as needed

vector<string> getWords(string s) {
    vector<string> res;
    res.reserve(10);
    int pos = 0;
    while (pos < s.size()) {
        pos = s.find(",");
        res.push_back(s.substr(0, pos));
        s.erase(0, pos + 1); // Length of delimiter is 1
    }
    return res;
}

string getCurrentDateTime() {
    auto now = chrono::system_clock::now();
    time_t now_c = chrono::system_clock::to_time_t(now);
    struct tm parts;
    localtime_s(&parts, &now_c);

    stringstream ss;
    ss << put_time(&parts, "%Y%m%d-%H%M%S");
    auto milliseconds = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;
    ss << "." << setfill('0') << setw(3) << milliseconds.count();
    return ss.str();
}

int CheckValidity(string orderID, string ClOrdID, string Instrument, int side, int qty, int price, ofstream& outputFile) {
    int flag = 0;
    while (flag == 0) {

        // Finding Invalid Instruments
        string arr[] = { "Rose","Lavender","Lotus","Tulip","Orchid" };
        bool exists = find(arr, arr + 5, Instrument) != arr + 5;
        if (!exists) {
            outputFile << orderID << "," << ClOrdID << "," << Instrument
                << "," << side << ",Reject," << qty
                << "," << price << "," << "Invalid Instrument" << "," << getCurrentDateTime() << "\n";
            flag = 1;
            break;
        }

        // Finding invalid side
        int arr2[] = { 1,2 };
        bool exist = find(arr2, arr2 + 2, side) != arr2 + 2;
        if (!exist) {
            outputFile << orderID << "," << ClOrdID << "," << Instrument
                << "," << side << ",Reject," << qty
                << "," << price << "," << "Invalid Side" << "," << getCurrentDateTime() << "\n";
            flag = 1;
            break;
        }

        // Is price positive?
        if (price <= 0) {

            outputFile << orderID << "," << ClOrdID << "," << Instrument
                << "," << side << ",Reject," << qty
                << "," << price << "," << "Invalid Price" << "," << getCurrentDateTime() << "\n";
            flag = 1;
            break;
        }

        // Is quantity a multiple of 10 and within the given range?
        int rem = qty % 10;
        if (rem != 0 || (qty > 1000) || (qty < 10)) {
            outputFile << orderID << "," << ClOrdID << "," << Instrument
                << "," << side << ",Reject," << qty
                << "," << price << "," << "Invalid Size" << "," << getCurrentDateTime() << "\n";
            flag = 1;
            break;
        }
        flag = 2;
    }
    return flag;
}

class OrderTable {
public:
    OrderTable() {}

    void insertRow(const std::string& orderID, const std::string& clientOrder, const std::string& instrument,
        int side, int qty, int price, ofstream& outputFile) {
        OrderRow row;
        row.orderID = orderID;
        row.clientOrder = clientOrder;
        row.side = side;
        row.execStatus = "New";
        row.qty = qty;
        row.price = price;
        row.reason = " ";
        row.instrument = instrument;

        OrderRow temp;

        if (side == 1) {
            int count = 0;
            if (sellTable_.empty()) {
                row.time = getCurrentDateTime();
                writetoFile(outputFile, row);
            }
            row.execStatus = "Rem";
            buyTable_.push(row);
            // Check if there are matching sellTable orders
            while (!sellTable_.empty()) {
                OrderRow sellRow = sellTable_.top();
                if (sellRow.qty != 0) {
                    if (row.price >= sellRow.price) {
                        count += 1;
                        if (row.qty == sellRow.qty) {
                            // Update the corresponding buyTable order
                            row.execStatus = "Fill";
                            row.price = sellRow.price;
                            row.time = getCurrentDateTime();
                            writetoFile(outputFile, row);
                            row.qty = 0;

                            // Update sellTable
                            sellRow.execStatus = "Fill";
                            sellRow.time = getCurrentDateTime();
                            writetoFile(outputFile, sellRow);

                            sellTable_.pop(); // Remove the matching sellTable order
                            break;
                        }
                        else if (row.qty > sellRow.qty) {
                            // Update the corresponding buyTable order
                            // Add the PFill row
                            row.execStatus = "PFill";
                            int remainder = row.qty - sellRow.qty;
                            int init_price = row.price;
                            row.qty = sellRow.qty;
                            row.price = sellRow.price;
                            row.time = getCurrentDateTime();
                            writetoFile(outputFile, row);

                            // Add the remainder row
                            temp = row;
                            buyTable_.pop(); // Remove the matching buyTable order
                            temp.qty = remainder;
                            temp.price = init_price;
                            temp.execStatus = "Rem";
                            buyTable_.push(temp);
                            row = temp;

                            // Update sellTable
                            sellRow.execStatus = "Fill";
                            sellRow.price = row.price;
                            sellRow.time = getCurrentDateTime();
                            writetoFile(outputFile, sellRow);
                            sellTable_.pop();
                        }
                        else {
                            // Update the corresponding buyTable order
                            row.execStatus = "Fill";
                            writetoFile(outputFile, row);
                            buyTable_.pop();

                            // Update sellTable
                            int remainder = sellRow.qty - row.qty;
                            int init_price = sellRow.price;
                            sellRow.execStatus = "PFill";
                            sellRow.qty = row.qty;
                            sellRow.price = row.price;
                            sellRow.time = getCurrentDateTime();
                            writetoFile(outputFile, sellRow);
                            sellRow.execStatus = "Rem";
                            sellRow.qty = remainder;
                            break;
                        }
                    }
                    else if (row.price < sellRow.price && count == 0) {
                        row.execStatus = "New";
                        row.time = getCurrentDateTime();
                        writetoFile(outputFile, row);
                        row.execStatus = "Rem";
                        break;
                    }
                    else {
                        break; // No more matching sellTable orders
                    }
                }
            }
        }
        else if (side == 2) {
            int count = 0;
            if (buyTable_.empty()) {
                row.time = getCurrentDateTime();
                writetoFile(outputFile, row);
            }
            row.execStatus = "Rem";
            sellTable_.push(row);
            // Check if there are matching buyTable orders
            while (!buyTable_.empty()) {
                OrderRow buyRow = buyTable_.top();
                if (buyRow.qty != 0) {
                    if (row.price <= buyRow.price) {
                        count += 1;
                        if (row.qty == buyRow.qty) {
                            // Update the corresponding sellTable order
                            row.execStatus = "Fill";
                            row.time = getCurrentDateTime();
                            writetoFile(outputFile, row);
                            sellTable_.pop();

                            // Update buyTable
                            buyRow.execStatus = "Fill";
                            buyRow.price = row.price;
                            buyRow.time = getCurrentDateTime();
                            writetoFile(outputFile, buyRow);

                            buyTable_.pop(); // Remove the matching buyTable order
                            break;
                        }
                        else if (row.qty > buyRow.qty) {
                            // Update the corresponding sellTable order
                            // Add the PFill row
                            int remainder = row.qty - buyRow.qty;
                            int init_price = row.price;
                            row.execStatus = "PFill";
                            row.qty = buyRow.qty;
                            row.price = buyRow.price;
                            row.time = getCurrentDateTime();
                            writetoFile(outputFile, row);

                            // Add the remainder row
                            temp = row;
                            sellTable_.pop(); // Remove the matching sellTable order
                            temp.qty = remainder;
                            temp.price = init_price;
                            temp.execStatus = "Rem";
                            sellTable_.push(temp);
                            row = temp;

                            // Update buyTable
                            buyRow.execStatus = "Fill";
                            buyRow.time = getCurrentDateTime();
                            writetoFile(outputFile, buyRow);
                            buyTable_.pop();
                        }
                        else {
                            // Update the corresponding sellTable order
                            row.execStatus = "Fill";
                            row.time = getCurrentDateTime();
                            writetoFile(outputFile, row);
                            sellTable_.pop();

                            // Update buyTable
                            int remainder = buyRow.qty - row.qty;
                            buyRow.execStatus = "PFill";
                            buyRow.qty = row.qty;
                            buyRow.time = getCurrentDateTime();
                            writetoFile(outputFile, buyRow);
                            buyRow.execStatus = "Rem";
                            buyRow.qty = remainder;
                            break;
                        }
                    }
                    else if (row.price < buyRow.price && count == 0) {
                        row.execStatus = "New";
                        row.time = getCurrentDateTime();
                        writetoFile(outputFile, row);
                        row.execStatus = "Rem";
                        break;
                    }
                    else {
                        break; // No more matching buyTable orders
                    }
                }
            }
        }
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
        string time;
        string reason;
    

    bool operator>(const OrderRow& other) const {
        return price > other.price;
    }

    };

    void writetoFile(ofstream& outputFile, const OrderRow& row) {
        outputFile << row.orderID << "," << row.clientOrder << "," << row.instrument
            << "," << row.side << "," << row.execStatus << "," << row.qty
            << "," << row.price << "," << row.reason << "," << row.time << "\n";
    }

    priority_queue<OrderRow, vector<OrderRow>, greater<OrderRow>> buyTable_;
    priority_queue<OrderRow, vector<OrderRow>, greater<OrderRow>> sellTable_;
};

int main() {
    ofstream MyFile("output.csv"); // File for output
    MyFile << "Order ID,Cl. Ord. ID,Instrument,Side,Exec Status,Quantity,Price,Reason,Execution Time" << endl; // Output file heading

    ifstream file; // File to be read
    file.open("order6.csv");
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

    vector<string> batchLines;
    int batchLineCounter = 0;

    // Reading an entire row
    getline(file, line);
    // Reading and processing the CSV file in batches
    while (getline(file, line)) {
        batchLines.push_back(line);
        batchLineCounter++;
        cout<<line[0]<<endl;

        if (batchLineCounter >= BATCH_SIZE || file.eof()) {
            // Process the batch of lines
            for (const string batchLine : batchLines) {
                words = getWords(batchLine);
                Ord_cnt += 1;

                string OrderID = "ord" + to_string(Ord_cnt);
                string Instrument = words[1];
                
                int flag = CheckValidity(OrderID, words[0], Instrument, stoi(words[2]), stoi(words[3]), stoi(words[4]), MyFile);
                if (flag == 1) {
                    continue;
                }

                if (orderTables.find(Instrument) != orderTables.end()) {
                    orderTables[Instrument].insertRow(OrderID, words[0], Instrument, stoi(words[2]), stoi(words[3]), stoi(words[4]), MyFile);
                }
            }

            // Clear the batchLines vector for the next batch
            batchLines.clear();
            batchLineCounter = 0;
        }
    }

    MyFile.close();
    file.close();
    return 0;
}

