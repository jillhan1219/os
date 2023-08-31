# Open the input file and read in the matrix size and values
with open("src.txt") as f:
    n = int(f.readline().strip())
    matrix = []
    for line in f:
        matrix.append([int(x) for x in line.strip().split()])

# Initialize the result matrix with all zeros
result = [[0 for j in range(n)] for i in range(n)]

# Perform matrix multiplication
for i in range(n):
    for j in range(n):
        for k in range(n):
            result[i][j] += matrix[i][k] * matrix[k][j]

# Print the result matrix
for i in range(n):
    for j in range(n):
        print(result[i][j], end=" ")
    print()
