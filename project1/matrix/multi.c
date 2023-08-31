#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_SIZE 4096
#define MAX_THREADS 16

int n, num_threads;
int A[MAX_SIZE][MAX_SIZE], B[MAX_SIZE][MAX_SIZE], C[MAX_SIZE][MAX_SIZE];
pthread_t threads[MAX_THREADS];

// Function to perform matrix multiplication on a partition of the matrices
void *multiply(void *arg)
{
    int thread_id = *(int *)arg;
    int start = thread_id * n / num_threads;
    int end = (thread_id + 1) * n / num_threads;

    for (int i = start; i < end; i++)
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

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // printf("Enter matrix size (must be a power of 2): ");
    // scanf("%d", &n);

    // printf("Enter number of threads (must be a power of 2): ");
    // scanf("%d", &num_threads);

    // Initialize matrices A and B
    if (argc == 1)
    {
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
    }
    else if (argc == 2)
    {
        n = atoi(argv[1]);
        // Initialize matrices A and B
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                A[i][j] = rand() % 10;
                B[i][j] = rand() % 10;
            }
        }
    }
    else
    {
        fprintf(stderr, "useage:%s <size> or %s\n", argv[0]);
        exit(1);
    }

    printf("Enter number of threads (must be a power of 2): ");
    scanf("%d", &num_threads);
    // Create threads to perform matrix multiplication on partitions of the matrices
    int thread_ids[MAX_THREADS];
    for (int i = 0; i < num_threads; i++)
    {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, multiply, (void *)&thread_ids[i]);
    }

    // Wait for threads to finish
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    FILE *data_out;
    if (argc == 1)
    {
        data_out = fopen("result.out", "w+");
    }
    else
    {
        data_out = fopen("random.out", "w+");
    }

    if (data_out == NULL)
    {
        printf("Failed to open out file\n");
        return 1;
    }

    if (argc == 2)
    {
        // print n A B
        fprintf(data_out, "%d\n", n);
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                fprintf(data_out, "%d ", A[i][j]);
            }
            fprintf(data_out, "\n");
        }

        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                fprintf(data_out, "%d ", B[i][j]);
            }
            fprintf(data_out, "\n");
        }
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
