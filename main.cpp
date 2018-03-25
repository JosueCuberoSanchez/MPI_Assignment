#include <mpi.h>
#include <stdio.h>
#include <math.h>

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
        //crear estructuras de datos
        //asignar trabajos
        cout << "SOY 0" << endl;
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

