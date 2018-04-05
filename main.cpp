#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string.h>
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

    srand (time(NULL)); //initialize random seed

    int myid, numprocs, namelen, n, tp;
    int *M, *Q, *CP, *P, *B, *M_Slice_B, *sendcounts_B, *displs_B;
    double startwtime, startwInputtime, endwInputtime, endwtime;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    MPI_Get_processor_name(processor_name,&namelen);

    MPI_Barrier(MPI_COMM_WORLD);

    if (myid == 0) {
        startwtime = MPI_Wtime();
        cout << "Digite el n" << endl;
        cin >> n;

        if(n%numprocs != 0){
            cout << "El numero de procesos debe ser multiplo de n" << endl;
            return 0;
        } else if(n == 0){
            cout << "Error de asignación de memoria" << endl;
            return 0;
        }

        startwInputtime = MPI_Wtime();

        M = new int[n*n];
        Q = new int[n];
        CP = new int[n*n];
        P = new int[n];
        B = new int[n*n];
        if(M == 0 || Q == 0 || CP == 0 || P == 0 || B == 0){
            cout << "Error de asignación de memoria" << endl;
            return 0;
        }

        for(int i=0;i<n;i++){
            for(int j=0;j<n;j++){
                M[i*n + j] = rand() % 9 + 1; //fill M with random ints from 0 to 9
            }
            P[i] = 0;
        }

        //Initialize the arrays for the Scatter
        sendcounts_B = new int[numprocs];
        if(sendcounts_B == 0){
            cout << "Error de asignación de memoria" << endl;
            return 0;
        }
        displs_B = new int[numprocs];
        if(displs_B == 0){
            cout << "Error de asignación de memoria" << endl;
            return 0;
        }

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
            displs_B[i] = (i * (n * n / numprocs)) - n;
        }
        // Taking out one row for the borders of the matrix
        // Putting a displacement of 0 for the first row
        displs_B[0] = 0;
        sendcounts_B[0] -= n;
        sendcounts_B[numprocs-1] -= n;
    }

    // Initialize Vector V with random ints from 0 to 5
    int V[n];
    if(V == 0){
        cout << "Error de asignación de memoria" << endl;
        return 0;
    }
    if (myid == 0) {
        for(int i=0;i<n;i++){
            V[i] = rand() % 4 + 1;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD); //other processes should wait for process 0 to set all structures
    MPI_Bcast(V, n, MPI_INT, 0, MPI_COMM_WORLD);

    // Size of dividing the matrix's dimension by the number of processes,
    // plus an extra row for the ones either on the top or bottom rows,
    // and plus two extra rows for the other rows
    //cout << "Soy el PROCESO " << myid << endl;
    if (myid == 0 || myid == n-1) {
        M_Slice_B = new int[(n*n/numprocs) + n];
    }
    else {
        M_Slice_B = new int[(n*n/numprocs) + (2*n)];
    }
    if(M_Slice_B == 0 || n == 0 || V == 0){
        cout << "Error de asignación de memoria" << endl;
        return 0;
    }
    MPI_Scatterv(M, sendcounts_B, displs_B, MPI_INT, M_Slice_B, n*n, MPI_INT, 0, MPI_COMM_WORLD);

    int rows = n/numprocs; //number of rows per process
    int columnPrimesToSend[n*rows]; //array to send if the column number is a prime
    int multiplicationResultsToSend[rows]; //results of array multiplication
    int BToSend[n*rows]; //rows to build B on root
    int tpToSend = 0; //total prime numbers
    int number, limit, start;
    if(myid == 0) {
        limit = rows;
        start = 0;
    } else {
        limit = rows + 1;
        start = 1;
    }
    if(rows == 0 || columnPrimesToSend == 0 || multiplicationResultsToSend == 0 ||
            BToSend == 0 || limit == 0){
        cout << "Error de asignación de memoria" << endl;
        return 0;
    }
    for(int i=0;i<rows;i++){ //initialize results vector in order to do += later
        multiplicationResultsToSend[i] = 0;
    }
    for (int i=start;i<limit;i++) { //process and calculate all
        for (int j = 0; j < n; j++) {
            number = M_Slice_B[i*n+j];
            if(number == 0){
                cout << "Error de asignación de memoria" << endl;
                return 0;
            }
            if (isPrime(number)) {
                tpToSend++;
                columnPrimesToSend[(i-start)*n+j] = 1;
            } else {
                columnPrimesToSend[(i-start)*n+j] = 0;
            }
            multiplicationResultsToSend[i-start] += (number*V[j]);
            BToSend[(i-start)*n+j] = number;
            if (((i*n+j)%n)>0) { //M[i,j-1], If is not in a left border
                BToSend[(i-start)*n+j] += M_Slice_B[(i*n+j)-1];
            }
            if ((i*n+j)>=n) {  //M[i-1,j], If is not in the top border
                BToSend[(i-start)*n+j] += M_Slice_B[(i*n+j)-n];
            }
            if ((i*n+j)%n!=(n-1)) {  //M[i,j+1], If is not in the right border
                BToSend[(i-start)*n+j] += M_Slice_B[(i*n+j)+1];
            }
            if ((((n*n)-(i*n+j))>n)&&(myid != n-1)){  //M[i+1,j], If is not in the down border
                BToSend[(i-start)*n+j] += M_Slice_B[(i*n+j)+n];
            }
        }
    }

    MPI_Reduce(&tpToSend, &tp, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Gather(columnPrimesToSend, n*rows, MPI_INT, CP, n*rows, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(BToSend, n*rows, MPI_INT, B, n*rows, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(multiplicationResultsToSend, rows, MPI_INT, Q, rows, MPI_INT, 0, MPI_COMM_WORLD);

    if (myid == 0) {

        for (int i = 0; i < n; ++i) { //fill P
            for (int j = 0; j < n; ++j) {
                if(CP[i*n+j]==1){ //it was a prime number
                    P[j]++;
                }
            }
        }

        endwInputtime = MPI_Wtime();
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
        cout << "********************" << endl <<  "Tiempos" << endl << "********************" << endl << endl;
        cout << "  Tiempo total de ejecución: " << (endwtime-startwtime) << " segundos" << endl;
        cout << "  Tiempo total sin desplegar datos: " << (endwInputtime-startwInputtime) << " segundos" << endl << endl;
        cout << "====================" << endl <<  "FIN DE LA EJECUCIÓN" << endl << "====================" << endl;
        fflush( stdout );
    }

    MPI_Finalize();
    return 0;
}

bool isPrime(int number){
    bool isPrime = true;
    if (number == 1 || number == 2 || number == 3) {
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