/*  
Autor: Branislav Mateas
Kontakt: xmatea00@stud.fit.vutbr.cz
Predmet Paralelní a distribuované algoritmy
Zadanie: 2. projekt - Mesh Multiplication
Rok: 2020/2021
*/
#include <iostream>
#include <cstdio>
#include <mpi.h>
#include <fstream>
#include <string>
#include <vector>

#define TAG_TO_BOTTOM 0
#define TAG_FROM_TOP 0 

#define TAG_TO_RIGHT 1
#define TAG_FROM_LEFT 1

/* FUNKCIE get_*_rank pre aktuálne ID procesory vrátia na základe výšky a šírky mriežky ID príslušných susedov*/
int get_right_rank(int rank, int width)
{
    return( ((rank+1) % width) ? (rank+1) : -1 ); 
}

int get_left_rank(int rank, int width)
{
    return((rank % width) ? (rank-1) : 0 );
}

int get_top_rank(int rank, int width)
{
    if(rank-width >= 0)
    {
        return (rank-width);
    }
    else
    {
        return 0;
    }

}

int get_bot_rank(int rank, int height, int width)
{
    if((rank+width) <= (height*width)-1)
    {
        return (rank+width);
    }
    else
    {
        return -1;
    }
}

/*Funkcia rozdeli retazec na vector podretazcov podla zadaneho oddelovaca */
std::vector<std::string> split(const std::string& str, const std::string& delim)
{
    using namespace std;
    vector<string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == string::npos) pos = str.length();
        string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}

/* Funkcia nacita po riadkoch vstup zo suboru obsahujuceho maticu */
int load_mat(std::ifstream &fp, std::vector<std::vector<int>>&mat)
{
    using namespace std;
    int ret;
    int tmp = 0;
    int val = 0;
    for(string line; getline(fp, line);)
    {
        vector<int> row;
        if(tmp == 0)
        {
            sscanf(line.c_str(), "%d", &ret); 
            tmp++;
            continue;
        }
        auto tokens = split(line, " ");
        for(int j = 0; j < tokens.size(); j++)
        {
            sscanf(tokens[j].c_str(), "%d", &val);
            row.push_back(val);
        }
        mat.push_back(row);
    }
    return ret;
}

/* Funkcia pre vypis matice na stdout */
void dump_matrix(std::vector<std::vector<int>> mat)
{
    using namespace std;
    for(int i = 0; i < mat.size(); i++)
    {
        for(int j = 0; j < mat[i].size();j ++)
        {
            cout << mat[i][j]  << " ";
        }
        cout << endl;
    }
}

int main(int argc, char** argv)
{
    using namespace std;
    int rank; 
    int proc_cnt;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_cnt);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  

    if(rank == 0)
    {
        MPI_Status stat;
        ifstream fpA("mat1");
        ifstream fpB("mat2");
        vector<vector<int>> matA;
        vector<vector<int>> matB; 

        /*Ziskanie mat1*/
        int rows = load_mat(fpA, matA);
        int matArows = matA.size();
        int matAcols = matA[0].size();

        /*Ziskanie mat2*/
        int cols = load_mat(fpB, matB);
        int matBrows = matB.size();
        int matBcols = matB[0].size();

        /*Kontrola maticeB*/
        for(int i = 0; i < matB.size(); i++)
        {
            if(matB[i].size() != cols)
            {
                MPI_Finalize();
                cout << "Zly format vstupu\nHint: medzi cislami matice je ocakavana medzera\n";
                return 1;
            }
        }
        /*Kontrola maticeA*/
        for(int i = 0; i < matA.size(); i++)
        {
            if(matA[i].size() != matBrows)
            {
                MPI_Finalize();
                cout << "Zly format vstupu\nHint: medzi cislami matice je ocakavana medzera\n";
                return 1;
            }
        }

        int width = matBcols;
        int height = matArows;
        int prod = 0;
        /*Rozposlanie rozmerov mriezky ostatnym procesorom*/
        for(int i = 0; i < proc_cnt; i++)
        {
            MPI_Send(&matArows, 1, MPI_INT, i, 4, MPI_COMM_WORLD);
            MPI_Send(&matAcols, 1, MPI_INT, i, 5, MPI_COMM_WORLD);
            MPI_Send(&matBcols, 1, MPI_INT, i, 6, MPI_COMM_WORLD);
        }

        int bottom = get_bot_rank(rank, height, width);
        int right = get_right_rank(rank, width);

        for(int i = 0; i < matAcols; i++)
        {
            
            int valA = matA[0][matAcols-i-1];
            int valB = matB[matAcols-i-1][0];
            prod += valA * valB;
            MPI_Send(&valA, 1, MPI_INT, right, TAG_TO_RIGHT, MPI_COMM_WORLD);
            MPI_Send(&valB, 1, MPI_INT, bottom, TAG_TO_BOTTOM, MPI_COMM_WORLD);
            int tmp = bottom;
            for(int j = 1; j < matArows; j++)
            {
                int val = matA[j][matAcols-i-1];
                MPI_Send(&val, 1, MPI_INT, tmp , TAG_TO_RIGHT, MPI_COMM_WORLD);
                tmp += width;
            }
            tmp = right-1;
            for(int k = 1; k < matBcols; k++)
            {
                int val = matB[matAcols-i-1][k];
                MPI_Send(&val, 1, MPI_INT, tmp+k, TAG_TO_BOTTOM , MPI_COMM_WORLD);
            }
        }
        cout << rows << ":" << cols << endl;
        cout << prod << " ";
        int collect;
        for(int i = 1; i < proc_cnt; i++)
        {
            MPI_Recv(&collect, 1, MPI_INT, i, 500, MPI_COMM_WORLD, &stat);
            cout << collect << " " << " \n"[((i+1)%width) == 0];
        } 
    }
    else
    {
        int l_matArows;
        int l_matAcols;
        int l_matBcols;
        MPI_Status stat;
        MPI_Recv(&l_matArows, 1, MPI_INT, 0, 4, MPI_COMM_WORLD, &stat);
        MPI_Recv(&l_matAcols, 1, MPI_INT, 0, 5, MPI_COMM_WORLD, &stat);
        MPI_Recv(&l_matBcols, 1, MPI_INT, 0, 6, MPI_COMM_WORLD, &stat);

        int valA, valB;
        int height = l_matArows;
        int width = l_matBcols;
        int top = get_top_rank(rank,  width);
        int bot = get_bot_rank(rank,  height, width);
        int left = get_left_rank(rank, width);
        int right = get_right_rank(rank, width);

        int prod = 0;
        for(int i = 0; i < l_matAcols; i++)
        {
            
            MPI_Recv(&valA, 1, MPI_INT, left, TAG_FROM_LEFT, MPI_COMM_WORLD, &stat);
            MPI_Recv(&valB, 1, MPI_INT, top, TAG_FROM_TOP, MPI_COMM_WORLD, &stat);

            prod += valA * valB;

            if(right != -1)
            {
                MPI_Send(&valA, 1, MPI_INT, right, TAG_TO_RIGHT, MPI_COMM_WORLD);
            }
            if(bot != -1)
            {
                MPI_Send(&valB, 1, MPI_INT, bot, TAG_TO_BOTTOM, MPI_COMM_WORLD);
            }
        }
        MPI_Send(&prod, 1, MPI_INT, 0,500, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}