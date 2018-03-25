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
        //data structures in general
        int n; //n entered by the user
        cout << "Digite el n" << endl;
        cin >> n;
        int M[n][n];
        int V[n];
        int Q[n];
        int P[n];
        int B[n][n];
        int tp;
        //data structures used by processes to calculate and store temporal data
        bool columnPrimes[n]; //boolean vector to store if the ieth element is prime
        int multiplicationResult; //result that will be used to create Q
        int tpProcess; //total of primes for a specified process

        for(int i=0;i<n;i++){
            for(int j=0;j<n;j++){
                M[i][j] = rand() % 9 + 1; //fill M with random ints from 0 to 9
            }
            V[i] = rand() % 4 + 1; //fill V with random ints from 0 to 5
        }

        //do scatter

    }

    //hacer operaciones


    if (myid == 0) {
        //procesar respuestas
        endwtime = MPI_Wtime();
        fflush( stdout );
    }

    MPI_Finalize();
    return 0;
}

