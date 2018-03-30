#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>

using namespace std;

bool isPrime(int number);

int main(int argc,char **argv) {
    int myid, numprocs, numthreads;
    int n; //n entered by the user
    double startwtime, endwtime;
    int  namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    // Arrays for the Scatter
    int* sendcounts_B;
    int* displs_B;
    int* M;
    int* V;

    MPI_Init(&argc,&argv);

    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);

    MPI_Comm_rank(MPI_COMM_WORLD,&myid);

    MPI_Get_processor_name(processor_name,&namelen);

    fprintf(stdout,"Proceso %d de %d en %s\n", myid, numprocs, processor_name);

    MPI_Barrier(MPI_COMM_WORLD);

    if (myid == 0) {
        int* array; //ejemplo
        array=new int[10];
        cout << "Digite el n" << endl;
        cin >> n;
        M = new int[n*n];
        V = new int[n];
        int Q[n];
        int P[n];
        int B[n*n];
        int tp;
        //data structures used by processes to calculate and store temporal data
        int columnPrimes[n]; //boolean vector to store if the ieth element is prime
        int multiplicationResult; //result that will be used to create Q
        int tpProcess; //total of primes for a specified process

        for(int i=0;i<n;i++){
            for(int j=0;j<n;j++){
                M[i*n + j] = rand() % 9 + 1; //fill M with random ints from 0 to 9
            }
            V[i] = rand() % 4 + 1; //fill V with random ints from 0 to 5
        }

        //Initialize the arrays for the Scatter
        sendcounts_B = new int[numprocs];
        displs_B = new int[numprocs];

        //  Each process will send an amount of rows based on the result of
        //  dividing the matrix's dimension by the number of processes plus
        //  one or two extra rows (depending on whether it is on eihter the top
        //  or bottom row of the matrix or not, respectively)
        //  The displacement is going to be similar, while also being
        //  multiplied by their entry's index number, which represents each row.
        //  It will have a row less of displacement, that in order to send the
        //  previous row as well.
        for (int i = 0 ; i < numprocs ; i++) {
            sendcounts_B[i]= ((n*n)/numprocs)+(2*n);
            displs_B[i] = (((n*n)/numprocs)-n)*i;
        }
        // Taking out one row for the borders of the matrix
        // Putting a displacement of 0 for the first row
        sendcounts_B[0] -= n;
        sendcounts_B[n-1] -= n;
        displs_B[0] = 0;
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD); //other processes should wait for process 0 to set all structures
    MPI_Bcast(V, n, MPI_INT, 0, MPI_COMM_WORLD);

    /*do scatter: send n/p+[1|2] rows, V and n.*/
    //Array that represents a slice of matrix M, used to compute matrix B
    int *M_Slice_B;

    // Size of dividing the matrix's dimension by the number of processes,
    // plus an extra row for the ones either on the top or bottom rows,
    // and plus two extra rows for the other rows
    if (myid == 0 || myid == n-1) {
        M_Slice_B = new int[((n*n)/numprocs)+n];
    }
    else {
      M_Slice_B = new int[((n*n)/numprocs)+(2*n)];
    }
    MPI_Scatterv(M, sendcounts_B, displs_B, MPI_INT, M_Slice_B, n*n, MPI_INT, 0, MPI_COMM_WORLD);

    //The next data structures are tests, because process should receive them from root
    int rowsReceived = 3; //HARDCODED
    int rowNumbers[rowsReceived]; //row numbers.
    int nReceived = 10; //this is n, but root should send it to the process HARDCODED
    int numRows;
    if(rowNumbers[0] == 0 || rowNumbers[rowsReceived] == n){
        numRows = rowsReceived+1; //if it is first or last row, it will receive just an extra row.
    } else {
        numRows = rowsReceived+2; //if it is another row, it will receive 2 extra rows.
    }
    int rows[numRows];
    for(int i=0;i<nReceived;i++){ //this for is just for HARDCODED values
        for(int j=0;j<nReceived;j++){
            rows[i*nReceived + j] = rand() % 9 + 1;
        }
    }
    int vReceived[nReceived]; //V received
    //The next variables are built by processes and sent to root process. Variable row is sent too.
    bool columnPrimesToSend[numprocs/nReceived]; //array to send if the column number is a prime
    int multiplicationResults[rowsReceived]; //results of array multiplication
    int tpToSend = 0;
    int bRows[rowsReceived]; //this will hold results for B matrix
    int number;
    for(int i=0;i<nReceived;i++){ //process and calculate all
        for(int j=0;j<nReceived;j++){
            number = rows[i*nReceived + j];
            if(isPrime(number)){
                tpToSend++;
                columnPrimesToSend[i*nReceived + j] = true;
            } else {
                columnPrimesToSend[i*nReceived + j] = false;
            }
            //multiply vector by matrix rows pending
            //build B pending
        }
    }


    if (myid == 0) {
        //procesar respuestas
        endwtime = MPI_Wtime();
        fflush( stdout );
    }

    MPI_Finalize();
    return 0;
}

bool isPrime(int number){
    bool isPrime = true;
    if (number == 2 || number == 3) {
        isPrime = true;
    } else if((number % 2 == 0) || (number % 3 == 0)){
        isPrime = false;
    } else {
        int i = 5;
        int j = 2;
        while(i*i <= number){
            if(number % i == 0){
                isPrime = false;
            }
            i += j;
            j = 6 - j;
        }
    }
    return isPrime;
}
