#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>

using namespace std;

typedef struct rootSendingStruct { //struct that root process will send to other processes
    int n; //n given by user
    int rows[]; //corresponding rows
    int V[]; //V array
    int rowNumbers[]; //process numbers, not same as process id, because assignation could slow the performance
} rootSendingStruct;

typedef struct processSendingStruct { //struct that other processes will send to root
    int tp; //total prime numbers
    int rowNumbers[]; //same process numbers received
    int multiplicationResults[];
    int bRows[]; //rows for B matrix
    bool columnPrimes[];
} processSendingStruct;

bool isPrime(int number);

int main(int argc,char **argv) {
    int myid, numprocs, numthreads;
    int n; //n entered by the user
    double startwtime, endwtime;
    int  namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc,&argv);

    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);

    /* create a type for struct car*/ //Esta parte me falta bretearla, es crear los structs de arriba. Si le llega, la hace :p
    /*const int    nitems=2;
    int          blocklengths[2] = {1,1};
    MPI_Datatype types[2] = {MPI_INT, MPI_INT};
    MPI_Datatype mpi_processSendingStruct;
    MPI_Aint     offsets[2];

    offsets[0] = offsetof(processSendingStruct, tp);
    offsets[1] = offsetof(processSendingStruct, rowNumbers);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_processSendingStruct);
    MPI_Type_commit(&mpi_processSendingStruct);*/

    MPI_Comm_rank(MPI_COMM_WORLD,&myid);

    MPI_Get_processor_name(processor_name,&namelen);

    fprintf(stdout,"Proceso %d de %d en %s\n", myid, numprocs, processor_name);

    MPI_Barrier(MPI_COMM_WORLD);

    if (myid == 0) {
        cout << "Digite el n" << endl;
        cin >> n;
        int M[n*n];
        int V[n];
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

        //do scatter: send n/p+[1|2] rows, V and n.

    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD); //other processes should wait for process 0 to set all structures

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
