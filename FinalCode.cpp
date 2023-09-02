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
    OrderTable(int initialCapacity = 100) {
        buyTable_.reserve(initialCapacity);
        sellTable_.reserve(initialCapacity);
    }

    void insertRow(const std::string& orderID, const std::string& clientOrder, const std::string& instrument,
               int side, int qty, int price, ofstream& outputFile) {
        OrderRow row;
        row.orderID = orderID;
        row.clientOrder = clientOrder;
        row.instrument = instrument;
        row.side = side;
        row.execStatus = "New";
        row.qty = qty;
        row.price = price;

        OrderRow temp;

        if (side == 1) {
            int count = 0;
            if (sellTable_.size()==0){
                outputFile << row.orderID << "," << row.clientOrder << "," << row.instrument
                           << "," << row.side << "," << row.execStatus << "," << row.qty
                           << "," << row.price << "\n";
            }
            row.execStatus = "Rem";
            buyTable_.push_back(row);
            // Check if there are matching sellTable orders
            for (auto& sellRow : sellTable_) {
                if (sellRow.qty != 0){
                    if (row.price >= sellRow.price) {
                        count += 1;
                        if(row.qty == sellRow.qty){
                            // Update the corresponding buyTable order
                            row.execStatus = "Fill";
                            outputFile << row.orderID << "," << row.clientOrder << "," << row.instrument
                                       << "," << row.side << "," << row.execStatus << "," << row.qty
                                       << "," << row.price << "\n";
                            row.qty = 0;

                            //Update sellTable
                            sellRow.execStatus = "Fill"; 
                            sellRow.price = row.price;
                            outputFile << sellRow.orderID << "," << sellRow.clientOrder << "," << sellRow.instrument
                                       << "," << sellRow.side << "," << sellRow.execStatus << "," << sellRow.qty
                                       << "," << sellRow.price << "\n";

                            sellRow.qty = 0;

                            break; // Only update the first matching sellTable order
                        }
                        else if (row.qty > sellRow.qty){                        
                            // Update the corresponding buyTable order
                            //Add the PFill row
                            row.execStatus = "PFill";
                            int remainder = row.qty - sellRow.qty;
                            row.qty = sellRow.qty;
                            outputFile << row.orderID << "," << row.clientOrder << "," << row.instrument
                                       << "," << row.side << "," << row.execStatus << "," << row.qty
                                       << "," << row.price << "\n";

                            //Add the remainder row
                            temp = row;
                            buyTable_.erase(buyTable_.end());
                            temp.qty = remainder;
                            temp.execStatus = "Rem";
                            buyTable_.push_back(temp);
                            row = temp;

                            //Update sellTable
                            sellRow.execStatus = "Fill";
                            sellRow.price = row.price;
                            outputFile << sellRow.orderID << "," << sellRow.clientOrder << "," << sellRow.instrument
                                       << "," << sellRow.side << "," << sellRow.execStatus << "," << sellRow.qty
                                       << "," << sellRow.price << "\n";
                            sellRow.qty = 0;
                        }
                        else{
                            // Update the corresponding buyTable order
                            row.execStatus = "Fill";
                            outputFile << row.orderID << "," << row.clientOrder << "," << row.instrument
                                       << "," << row.side << "," << row.execStatus << "," << row.qty
                                       << "," << row.price << "\n";
                            
                            row.qty = 0;

                            //Update sellTable
                            int remainder = sellRow.qty - row.qty;
                            int init_price = sellRow.price;
                            sellRow.execStatus = "PFill";
                            sellRow.qty = row.qty;
                            sellRow.price = row.price;
                            outputFile << sellRow.orderID << "," << sellRow.clientOrder << "," << sellRow.instrument
                                       << "," << sellRow.side << "," << sellRow.execStatus << "," << sellRow.qty
                                       << "," << sellRow.price << "\n";
                            sellRow.execStatus = "Rem";
                            sellRow.qty = remainder;
                            sellRow.price = init_price;
                            break; // Only update the first matching sellTable order
                        }                        
                    }else if (row.price<sellRow.price & count == 0){
                        row.execStatus = "New";
                        outputFile << row.orderID << "," << row.clientOrder << "," << row.instrument
                                << "," << row.side << "," << row.execStatus << "," << row.qty
                                << "," << row.price << "\n";
                        row.execStatus = "Rem";
                        break;
                    }
                }
            }
            sort(buyTable_.begin(), buyTable_.end(), [](const OrderRow& a, const OrderRow& b) {
                return a.price > b.price;
            });
            
        } else if (side == 2) {
            int count = 0;
            if (buyTable_.size()==0){
                outputFile << row.orderID << "," << row.clientOrder << "," << row.instrument
                           << "," << row.side << "," << row.execStatus << "," << row.qty
                           << "," << row.price << "\n";
            }
            row.execStatus = "Rem";
            sellTable_.push_back(row);
            // Check if there are matching buyTable orders
            for (auto& buyRow : buyTable_) {
                if (buyRow.qty !=0){
                    if (row.price <= buyRow.price) {
                        count += 1;
                        if(row.qty==buyRow.qty){
                            // Update the corresponding sellTable order
                            row.execStatus = "Fill";
                            outputFile << row.orderID << "," << row.clientOrder << "," << row.instrument
                                       << "," << row.side << "," << row.execStatus << "," << row.qty
                                       << "," << row.price << "\n";
                            row.qty = 0;

                            //Update buyTable
                            buyRow.execStatus = "Fill"; 
                            buyRow.price = row.price;
                            outputFile << buyRow.orderID << "," << buyRow.clientOrder << "," << buyRow.instrument
                                       << "," << buyRow.side << "," << buyRow.execStatus << "," << buyRow.qty
                                       << "," << buyRow.price << "\n";

                            buyRow.qty = 0;

                            break; // Only update the first matching buyTable order
                        }
                        else if(row.qty>buyRow.qty){
                            // Update the corresponding sellTable order
                            //Add the PFill row
                            int remainder = row.qty - buyRow.qty;
                            int init_price = row.price;
                            row.execStatus = "PFill";
                            row.qty = buyRow.qty;
                            row.price = buyRow.price;
                            outputFile << row.orderID << "," << row.clientOrder << "," << row.instrument
                                       << "," << row.side << "," << row.execStatus << "," << row.qty
                                       << "," << row.price << "\n";

                            //Add the remainder row
                            temp = row;
                            sellTable_.erase(sellTable_.end());
                            temp.qty = remainder;
                            temp.price = init_price;
                            temp.execStatus = "Rem";
                            sellTable_.push_back(temp);
                            row = temp;

                            //Update buyTable
                            buyRow.execStatus = "Fill";
                            outputFile << buyRow.orderID << "," << buyRow.clientOrder << "," << buyRow.instrument
                                       << "," << buyRow.side << "," << buyRow.execStatus << "," << buyRow.qty
                                       << "," << buyRow.price << "\n";
                            buyRow.qty = 0;
                        }
                        else{
                            // Update the corresponding sellTable order
                            row.execStatus = "Fill";
                            outputFile << row.orderID << "," << row.clientOrder << "," << row.instrument
                                       << "," << row.side << "," << row.execStatus << "," << row.qty
                                       << "," << row.price << "\n";
                            
                            row.qty = 0;

                            //Update buyTable
                            int remainder = buyRow.qty - row.qty;
                            buyRow.execStatus = "PFill";
                            buyRow.qty = row.qty;
                            outputFile << buyRow.orderID << "," << buyRow.clientOrder << "," << buyRow.instrument
                                       << "," << buyRow.side << "," << buyRow.execStatus << "," << buyRow.qty
                                       << "," << buyRow.price << "\n";
                            buyRow.execStatus = "Rem";
                            buyRow.qty = remainder;
                            break; // Only update the first matching buyTable order
                        }
                    }else if (row.price<buyRow.price & count == 0){
                        row.execStatus = "New";
                        outputFile << row.orderID << "," << row.clientOrder << "," << row.instrument
                                << "," << row.side << "," << row.execStatus << "," << row.qty
                                << "," << row.price << "\n";
                        row.execStatus = "Rem";
                        break;
                    }
                }
            }            
            sort(sellTable_.begin(), sellTable_.end(), [](const OrderRow& a, const OrderRow& b) {
                return a.price < b.price;
            });
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
    };

    vector<OrderRow> buyTable_;
    vector<OrderRow> sellTable_;
};

int main() {
    ofstream MyFile("output.csv"); // File for output
    MyFile << "Order ID,Cl. Ord. ID,Instrument,Side,Exec Status,Quantity,Price" << endl; // Output file heading

    ifstream file; // File to be read
    file.open("order6.csv");
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
        orders.insertRow(OrderID, words[0], words[1], stoi(words[2]), stoi(words[3]), stoi(words[4]),MyFile);

    }

    MyFile.close();

    file.close();
    return 0;
}