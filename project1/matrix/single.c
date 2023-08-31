#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 4096

int n, num_threads;
int A[MAX_SIZE][MAX_SIZE], B[MAX_SIZE][MAX_SIZE], C[MAX_SIZE][MAX_SIZE];

int main(int argc, char *argv[])
{
    // printf("Enter matrix size (must be a power of 2): ");
    // scanf("%d", &n);

    // printf("Enter number of threads (must be a power of 2): ");
    // scanf("%d", &num_threads);

    // Initialize matrices A and B
    FILE *data_in = fopen("sample_data.in", "r");
    if (data_in == NULL)
    {
        printf("Failed to open in file\n");
        return 1;
    }

    fscanf(data_in, "%d", &n);

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            fscanf(data_in, "%d", &A[i][j]);
            B[i][j] = A[i][j];
        }
    }
    printf("input successfully\n");

    fclose(data_in);

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            C[i][j] = 0;
            for (int k = 0; k < n; k++)
            {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    FILE *data_out;
    data_out = fopen("result.out", "w+");

    if (data_out == NULL)
    {
        printf("Failed to open out file\n");
        return 1;
    }

    // print C
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            // printf("%d ", C[i][j]);
            fprintf(data_out, "%d ", C[i][j]);
        }
        // printf("\n");
        fprintf(data_out, "\n");
    }

    printf("output successfully\n");

    fclose(data_out);

    return 0;
}
