#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_SIZE 1024
#define MAX_THREADS 8

int n, num_threads;
int A[MAX_SIZE][MAX_SIZE], B[MAX_SIZE][MAX_SIZE], C[MAX_SIZE][MAX_SIZE];
pthread_t threads[MAX_THREADS];

// Function to perform matrix multiplication on a partition of the matrices
void* multiply(void* arg) {
    int thread_id = *(int*)arg;
    int start = thread_id * n / num_threads;
    int end = (thread_id + 1) * n / num_threads;

    for (int i = start; i < end; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0;
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    pthread_exit(NULL);
}

int main() {
    printf("Enter matrix size (must be a power of 2): ");
    scanf("%d", &n);

    printf("Enter number of threads (must be a power of 2): ");
    scanf("%d", &num_threads);

    // Initialize matrices A and B
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A[i][j] = rand() % 10;
            B[i][j] = rand() % 10;
        }
    }

    // Create threads to perform matrix multiplication on partitions of the matrices
    int thread_ids[MAX_THREADS];
    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, multiply, (void*)&thread_ids[i]);
    }

    // Wait for threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print the result matrix C
    printf("Result matrix C:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d ", C[i][j]);
        }
        printf("\n");
    }

    return 0;
}

/*
Q:when i'm writing a program of multi-threads, is it necessary to use `pthread_attr__t` and initialize it?
A:It depends on your specific use case and what behavior you want your threads to have.

pthread_attr_t is a structure used to specify attributes for the behavior of a thread. It can be used to set various properties such as the stack size, scheduling policy, and priority. By default, a thread is created with default attributes.

If you want to customize the attributes of your threads, then you should use pthread_attr_t and initialize it to set the desired attributes. However, if you are satisfied with the default behavior of threads, then it is not necessary to use pthread_attr_t.

So, the answer to your question is, it depends on your specific needs and requirements.

Q:what are the default attributes?
A:The default attributes for a pthread are:

Detached state: By default, threads are created in the joinable state, which means that other threads can wait for them to terminate using the pthread_join function. If you don't set the detached state attribute using pthread_attr_setdetachstate(), then the thread will be created in the joinable state by default.
Scheduling policy: By default, threads are created with the SCHED_OTHER scheduling policy, which means that they are scheduled according to the standard time-sharing algorithm.
Scheduling priority: By default, threads are created with a scheduling priority of 0.
Stack size: By default, threads are created with a system-dependent default stack size, which is typically several megabytes in size.
Thread-specific data: By default, threads don't have any thread-specific data associated with them.
Note that the default attributes may vary depending on the specific implementation of the pthreads library and the operating system you are using.

Q:If I set all the threads to detached state, do i need to use `pthread_join`?
A:No, if you set all the threads to the detached state using pthread_attr_setdetachstate(), you do not need to use pthread_join() to wait for them to complete.

When a thread is in the detached state, its resources are automatically released when it terminates, and the system will reclaim the resources without the need for another thread to call pthread_join() on it.

However, keep in mind that if your program relies on the order of execution of the threads or needs to synchronize their operations, then using the joinable state with pthread_join() may be necessary. In such cases, you would need to create threads in the joinable state using pthread_create() without calling pthread_attr_setdetachstate(), and then use pthread_join() to wait for them to complete.


*/

gnuplot
set title ''
set xlabel ''
set ylabel ''
set term pngcairo
plot 'sample.csv' using 1:2 "%lf,%lf"

//统计文件的数字数目
grep -o -E '[0-9]+' filename.in | wc -l
//获取绝对路径
realpath "文件名"

set terminal png
set output 'akamai.png'

set xdata time
set timefmt "%S"
set xlabel "time"

set autoscale

set ylabel "highest seq number"
set format y "%s"

set title "seq number over time"
set key reverse Left outside
set grid

set style data linespoints

plot "print_1012720" using 1:2 title "Flow 1", \
plot "print_1058167" using 1:2 title "Flow 2", \
plot "print_193548"  using 1:2 title "Flow 3", \ 
plot "print_401125"  using 1:2 title "Flow 4", \
plot "print_401275"  using 1:2 title "Flow 5", \
plot "print_401276"  using 1:2 title "Flow 6"

# 第10-100 行
plot "my.dat" every ::10::100


$ cd ~/Documents
$ script example.log
Script started, file is example.log
$ ls
Desktop  Documents  Downloads  Music  Pictures  Public  Templates  Videos
$ mkdir test
$ cd test
$ touch file1 file2 file3
$ ls
file1  file2  file3
$ exit
exit
Script done, file is example.log

script server_output.txt
./server

script client_output.txt
./client <server_ip_address> <port_number>
