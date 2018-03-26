#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>

using namespace std;

int main(int argc,char **argv)
{
    int myid, numprocs, numthreads, i;
    int n; //n entered by the user
    int M[n*n];
    int V[n];
    int Q[n];
    int P[n];
    int B[n*n];
    int tp;
    //data structures used by processes to calculate and store temporal data
    bool columnPrimes[n]; //boolean vector to store if the ieth element is prime
    int multiplicationResult; //result that will be used to create Q
    int tpProcess; //total of primes for a specified process

    double startwtime, endwtime;
    int  namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc,&argv);

    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);

    MPI_Comm_rank(MPI_COMM_WORLD,&myid);

    MPI_Get_processor_name(processor_name,&namelen);

    fprintf(stdout,"Proceso %d de %d en %s\n", myid, numprocs, processor_name);

    MPI_Barrier(MPI_COMM_WORLD);

    if (myid == 0) {
        cout << "Digite el n" << endl;
        cin >> n;

        for(int i=0;i<n;i++){
            for(int j=0;j<n;j++){
                M[i*n + j] = rand() % 9 + 1; //fill M with random ints from 0 to 9
            }
            V[i] = rand() % 4 + 1; //fill V with random ints from 0 to 5
        }

        //do scatter: send n/p+[1|2] rows, V and n.

    }

    //The next data structures are tests, because process should receive them from root
    int row; //row number
    int nReceived; //this is n, but root should send it to the process
    if(row == 0 || row == n){
        int rows[(numprocs/nReceived)+1]; //if it is first or last row, it will receive just an extra row.
    } else {
        int rows[(numprocs/nReceived)+2]; //if it is another row, it will receive 2 extra rows.
    }
    int vReceived[nReceived]; //V received
    //hacer operaciones


    if (myid == 0) {
        //procesar respuestas
        endwtime = MPI_Wtime();
        fflush( stdout );
    }

    MPI_Finalize();
    return 0;
}

