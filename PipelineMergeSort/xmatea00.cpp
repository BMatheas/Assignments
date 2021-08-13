#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

using namespace std;

int16_t getSmallest(queue<int16_t>& q0, queue<int16_t>& q1, int&flag)
{
    int16_t tmp;
    if(q0.empty())
    {
        if(q1.empty())
        {
            tmp = -1;
        }
        else
        {
            tmp = q1.front();
            q1.pop();
            flag = 1;
        }
    }
    else
    {
        if(q1.empty())
        {
            tmp = q0.front();
            q0.pop();
            flag = 0;
        }
        else
        {
            if(q0.front() < q1.front())
            {
                tmp = q0.front();
                q0.pop();
                flag = 0;
            }
            else
            {
                tmp = q1.front();
                q1.pop();
                flag = 1;
            }
        }
    }
    return tmp;
}

int main(int argc, char** argv)
{
    int rank, proc_cnt;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_cnt); 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0)
    {
        int16_t number;
        fstream fin;
        fin.open("numbers", ios::in);
        int j = 0;
        while(fin.good()) 
        {
            number = fin.get();
            if(!fin.good()) 
            {
                cout << endl;
                break;
            }
            cout << number << " ";
            MPI_Send(&number, 1, MPI_INT16_T, rank+1, j, MPI_COMM_WORLD);
            j = (j+1) % 2;
        }
        fin.close();
        number = -1;
        MPI_Send(&number, 1, MPI_INT16_T, rank+1, 3, MPI_COMM_WORLD);
    }
    else
    {
        queue<int16_t> q0;
        queue<int16_t> q1;  

        int available_size = pow(2, rank-1);
        int q0_available = available_size;
        int q1_available = available_size;

        MPI_Status status;
        int16_t number;
        int16_t qnum;
        int done = 0;
        int j = 0;
        int k = 1;
        int begin = 0;
        int m = 1;
        int flag = -1;
        while(k < pow(2, proc_cnt-1))
        {
            
            if(!done)
            {
                MPI_Recv(&number, 1, MPI_INT16_T,(rank-1), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                if(status.MPI_TAG == 0)
                {
                    q0.push(number);
                }
                else if(status.MPI_TAG == 1)
                {
                    q1.push(number);
                }
                else
                {
                    done = 1;
                }  
            }
            if(q0.size() == pow(2, rank-1) && q1.size() == 1) begin = 1;
            if(begin)
            {
                if(rank == (proc_cnt-1))
                {
                    cout << getSmallest(q0, q1, flag) << endl;
                    k++;
                }
                else
                {
                    if(q0_available ==0)
                    {
                        qnum = q1.front();q1.pop();
                        q1_available--;
                    }
                    else if(q1_available==0)
                    {
                        qnum = q0.front();q0.pop();
                        q0_available--;
                    }
                    else
                    {
                        qnum = getSmallest(q0,q1,flag);
                        switch(flag)
                        {
                            case 0 : 
                                q0_available--;
                                break;
                            case 1 :
                                q1_available--;
                                break;
                        }
                    }
                    MPI_Send(&qnum, 1, MPI_INT16_T, rank+1, j, MPI_COMM_WORLD);
                    m++;
                    if(m > pow(2, rank))
                    {
                        k = k + pow(2, rank);
                        j = (j+1) % 2;
                        m = 1;
                        q0_available = q1_available = available_size;
                    }
                }
            }
        }
        if(rank != (proc_cnt-1))
        {
            qnum = 1;
            MPI_Send(&qnum, 1, MPI_INT16_T, rank+1, 3, MPI_COMM_WORLD);
        }
    }
    MPI_Finalize();
    return 0;
}