#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <fstream>

using namespace std;

/**
* Josué Cubero (B4), Daniel González (B42991)
* Tarea Programada 1 - Arquitectura de Computadoras (CI-1323)
* UCR - I Semestre 2018
**/

bool isPrime(int number);

int main(int argc,char **argv) {
    int myid, numprocs, numthreads;
    int n=0; //n entered by the user
    double startwtime, startwInputtime, endwtime;
    srand (time(NULL)); //initialize random seed
    int  namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    // Arrays for the Scatter
    int* sendcounts_B;
    int* displs_B;
    int* M;
    int tp;

    MPI_Init(&argc,&argv);

    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);

    MPI_Comm_rank(MPI_COMM_WORLD,&myid);

    MPI_Get_processor_name(processor_name,&namelen);

    fprintf(stdout,"Proceso %d de %d en %s\n", myid, numprocs, processor_name);

    MPI_Barrier(MPI_COMM_WORLD);

    if (myid == 0) {
        startwtime = MPI_Wtime();
        int* array; //ejemplo
        array=new int[10];
        cout << "Digite el n" << endl;
        cin >> n;

        startwInputtime = MPI_Wtime();

        M = new int[n*n];

        for(int i=0;i<n;i++){
            for(int j=0;j<n;j++){
                M[i*n + j] = rand() % 9 + 1; //fill M with random ints from 0 to 9
            }
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
          sendcounts_B[i]= (n*n/numprocs) + (2*n);
          displs_B[i] = (i*(n*n/numprocs))-n;
        }
        // Taking out one row for the borders of the matrix
        // Putting a displacement of 0 for the first row
        sendcounts_B[0] -= n;
        sendcounts_B[n-1] -= n;
        displs_B[0] = 0;
    }

    int Q[n];
    int P[n];
    int B[n*n];

    //data structures used by processes to calculate and store temporal data
    int columnPrimes[n]; //boolean vector to store if the ieth element is prime
    int multiplicationResult; //result that will be used to create Q
    int tpProcess; //total of primes for a specified process

    // Initialize Vector V with random ints from 0 to 5
    int V[n];
    if (myid == 0) {
      for(int i=0;i<n;i++){
        V[i] = rand() % 4 + 1;
      }
      cout << "V: (" << myid << ")" <<  endl;
      for (int i=0; i<n; i+=1) {
          cout << V[i] << " ";
      }
      cout << endl << endl;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD); //other processes should wait for process 0 to set all structures
    MPI_Bcast(V, n, MPI_INT, 0, MPI_COMM_WORLD);


    /*do scatter: send n/p+[1|2] rows, V and n.*/
    //Array that represents a slice of matrix M, used to compute matrix B
    int *M_Slice_B;

    // Size of dividing the matrix's dimension by the number of processes,
    // plus an extra row for the ones either on the top or bottom rows,
    // and plus two extra rows for the other rows
    cout << "Soy el PROCESO " << myid << endl;
    if (myid == 0 || myid == n-1) {
        M_Slice_B = new int[(n*n/numprocs) + n];
    }
    else {
      M_Slice_B = new int[(n*n/numprocs) + (2*n)];
    }

    MPI_Scatterv(M, sendcounts_B, displs_B, MPI_INT, M_Slice_B, n*n, MPI_INT, 0, MPI_COMM_WORLD);


    // cout << "Filas de M recibidas:" << endl;
	  //  if (myid==0 || myid==numprocs-1) {
    // 		for (int i=0; i<n * n/numprocs + n; i++) {
    // 			if (i%n==0 && i!=0) {
    // 				cout << endl;
    // 			}
    //       cout << M_Slice_B[i] << " ";
    // 		}
    //     cout << endl;
    // 	}
    // 	else {
    // 		for (int i=0; i<n * n/numprocs + 2*n; i++) {
    // 			if (i%n==0 && i!=0) {
    // 				cout << endl;
    // 			}
    // 			cout << M_Slice_B[i] << " ";
    // 		}
    // 		cout << endl;
    // 	}

    //The next data structures are tests, because process should receive them from root
    int rowsReceived = 3; //HARDCODED
    int rowNumbers[rowsReceived]; //row numbers.
    int nReceived = n; //this is n, but root should send it to the process HARDCODED
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

    MPI_Barrier(MPI_COMM_WORLD);
    if (myid == 0) {
      //Prepare output files, if n was greater than 100, otherwise use stdout
        streambuf* outBufM;
        streambuf* outBufV;
        streambuf* outBufQ;
        streambuf* outBufP;
        streambuf* outBufB;
        ofstream ofM;
        ofstream ofV;
        ofstream ofQ;
        ofstream ofP;
        ofstream ofB;

        if (n > 100) {
          ofM.open("M.txt");
          ofV.open("V.txt");
          ofQ.open("Q.txt");
          ofP.open("P.txt");
          ofB.open("B.txt");
          outBufM = ofM.rdbuf();
          outBufV = ofV.rdbuf();
          outBufQ = ofQ.rdbuf();
          outBufP = ofP.rdbuf();
          outBufB = ofB.rdbuf();
        }
        else {
          outBufM = cout.rdbuf();
          outBufV = cout.rdbuf();
          outBufQ = cout.rdbuf();
          outBufP = cout.rdbuf();
          outBufB = cout.rdbuf();
        }
        ostream outM(outBufM);
        ostream outV(outBufV);
        ostream outQ(outBufQ);
        ostream outP(outBufP);
        ostream outB(outBufB);

        //Results
        cout << "====================" << endl <<  "FIN DE LA EJECUCIÓN" << endl << "====================" << endl;
        cout << "  Valor de n: " << n << endl;
        cout << "  Número de procesos: " << numprocs << endl << endl;

        cout << "  Número total de primos en M: " << tp << endl << endl;


        cout << "********************" << endl <<  "Matrices y Vectores" << endl << "********************" << endl << endl;

        outM << "Matriz M:" << endl << "----" << endl;
        for (int i=0; i<n; i+=1) {
            for (int j=0; j<n; j+=1) {
                outM << M[i*n+j] << " ";
            }
            outM << endl;
        }
        outM << endl;

        outV << "Vector V:" << endl << "----" << endl;
        for (int i=0; i<n; i+=1) {
            outV << V[i] << " ";
        }
        outV << endl << endl;

        outQ << "Vector Q:" << endl << "----" << endl;
        for (int i=0; i<n; i+=1) {
            outQ << Q[i] << " ";
        }
        outQ << endl << endl;

        outP << "Vector P:" << endl << "----" << endl;
        for (int i=0; i<n; i+=1) {
            outP << P[i] << " ";
        }
        outP << endl << endl;

        outB << "Matriz B:" << endl << "----" << endl;
        for (int i=0; i<n; i+=1) {
            for (int j=0; j<n; j+=1) {
                outB << B[i*n+j] << " ";
            }
            outB << endl;
        }
        outB << endl;

        if (n > 100) {
          ofM.close();
          ofV.close();
          ofQ.close();
          ofP.close();
          ofB.close();
          cout << "La información de Matrices y Vectores se encuentra\nen archivos de texto, según sus nombres." << endl;
        }

        endwtime = MPI_Wtime();
        cout << endl << "  Tiempo total de ejecución: " << (endwtime-startwtime) << endl;
        cout << "  Tiempo total desde que se ingresó n: " << (endwtime-startwInputtime) << endl << endl;
        cout << "====================" << endl <<  "FIN DE LA EJECUCIÓN" << endl << "====================" << endl;
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
