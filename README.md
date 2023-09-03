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

