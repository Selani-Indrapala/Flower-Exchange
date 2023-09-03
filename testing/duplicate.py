import csv

# Specify the input CSV file and output CSV file
input_file = "order6.csv"
output_file = "order7.csv"
num_duplicates = 3  # Change this to the number of times you want to duplicate the content
target_row_count = 10000  # Total number of rows in the output file

# Read data (including header) from the input CSV file
with open(input_file, mode="r", newline="") as input_csv:
    reader = csv.reader(input_csv)
    data = list(reader)

# Extract the header from the data
header = data[0]

# Determine how many times the data (excluding header) needs to be duplicated
data_without_header = data[1:]
data_row_count = len(data_without_header)
num_duplicates = (target_row_count - 1) // data_row_count  # Subtract 1 for the header

# Write data (including header) to the output CSV file until reaching the target row count
with open(output_file, mode="w", newline="") as output_csv:
    writer = csv.writer(output_csv)
    
    # Write the header row
    writer.writerow(header)
    
    # Write data (excluding header) to reach the target row count
    for _ in range(num_duplicates):
        writer.writerows(data_without_header)


        
print(f"Content from {input_file} has been duplicated {num_duplicates} times to {output_file}.")



