#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <typeinfo>

using namespace std;

vector<string> getWords(string s){
            vector<string> res;
            int pos = 0;
            while(pos < s.size()){
                pos = s.find(",");
                res.push_back(s.substr(0,pos));
                s.erase(0,pos+1); //Length of delimeter is 1
            }
            return res;
        }

class Flowers{
    public:

        int Price, Qty;
        string Instrument, ClientID;

        //Sell Side

        //Buy Side

        //Method to choose the side of the Order Book
        void side(int sd){
            if(sd==1){
                cout<<"Buy"<<endl;
            }
            else if (sd==2){
                cout<<"Sell"<<endl;
            }
            else{
                cout<<"Invalid"<<endl;
            }
        }
};
/*
class Rose: public Flowers{

};

class Lavender: public Flowers{

};

class Lotus: public Flowers{

};

class Tulip: public Flowers{

};

class Orchid: public Flowers{

};
*/
int main() {

    ofstream MyFile("filename.csv"); //File for output
    MyFile << "Order ID,Cl. Ord. ID,Instrument,Side,Exec Status,Quantity,Price"<<endl; //Output file heading

    ifstream file; //File to be read
    file.open("order2.csv");
    string line, sd;
    vector<string> words;

    //Reading an entire row
    getline(file, line);
    while (getline(file, line)) {
        words = getWords(line); //words is a vector containing the string of each attribute in a row 

        //Items of the order as per the Flower class
        Flowers order;
        order.ClientID = words[0];
        order.Instrument = words[1];
        order.side(stoi(words[2]));
        order.Qty = stoi(words[3]);
        order.Price = stoi(words[4]);
        //cout<<order.ClientID<<" "<<order.Instrument<<" "<<order.Qty<<" "<<order.Price<<endl;


        // Write to the file
        MyFile << "Order," << order.ClientID << "," << order.Instrument <<","<<  words[2] <<",exec,"<< order.Qty<<","<< order.Price <<endl;

    }
    MyFile.close();    

    file.close();
    return 0;
}
