# Flower-Exchange-LSEG
The code applicable for this project is the FinalCode_Optimized.cpp. When running the code, the csv test files should be stored within the same folder as the code.

The main functions of the code is explained below,

The getWords function takes in each line read by the csv and splits it into seperate words after each comma. The words are then put into a vector for later utilisation.

The getCurrentDateTime function is used to output the time at which a particular command is executed. This helps identify the time taken for the code to run and can be utilised for optimisation purposes.

To check the validity of the csv data, the CheckValidity function is used where the function rejects the data in the event that,

1. There is no data available.
2. The instrument is invalid.
3. The side is invalid.
4. The price is negative.
5. The quantity is not a multiple of 10 and does not lie within the range [10,1000].

The OrderTable is next created with both a sellTable and buyTable implemented using priority queues. Depending on the data read by the csv, the row is then added into either the sell or buy table. The corresponding other table is checked to see if any orders can be matched. Three cases are considered in the matching which are,

1. Both rows have an equal quantity in which case both are Filled.
2. The new row has a higher quantity making it partially filled and the other Filled.
3. The new row has a lower quantity making it Filled and the other partially filled.

The code was tested for the given csv cases given which can be found under the folder "order csv examples". And for large data sets, the given data sets were duplicated using an algorithm which can be found under the "Testing" folder. 

Different implementations of the code can be found under the "Old Code" folder. These implementations include vectors, priority queues, batch processing, etc. However, the final submission (FinalCode_Optimized.cpp) holds the code with the highest optimization.
