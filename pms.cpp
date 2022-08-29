/**
 * Pipeline Merge Sort
 * Autor: Dominik Švač (xsvacd00)
 */

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <mpi.h>
#include <math.h>
#include <deque>
#include <chrono>

#define TAG_FIRST_PIPELINE 0
#define TAG_SECOND_PIPELINE 1
#define TAG_END 3

/**
 * Vypíše hodnoty oddelené medzerov na jeden riadok
 * @param v zoznam hodnôt
 */
void printVector(const std::deque<int>& v)
{
    for (auto elem = v.begin(); elem != v.end(); elem++)
    {
       std::cout << *elem; 
        if (elem != (v.end()-1))
            std::cout << " ";
    }
    std::cout << std::endl << std::flush;
}

/**
 * Vypíše hodnoty od konca po začiatok, každý prvok na nový riadok
 * @param v zoznam hodnôt
 */
void printAtTheEnd(const std::deque<int>& v)
{
    for (auto elem = v.end()-1; elem != v.begin(); elem--)
       std::cout << *elem << std::endl;

    std::cout << *v.begin() << std::endl;
}

/**
 * @param deq1 zoznam hodnôt
 * @return vráti prvý prvok z množiny a zaroveň ho odstráni 
 */ 
int getFirst(std::deque<int>& deq1)
{
    auto elem = deq1.front();
    deq1.pop_front();
    return elem; 
}

/**
 * Vyberie sa z 2 množín väčšia hodnota a vymaže ju
 * @param deq1 množina hodnôt 1
 * @param deq2 množina hodnôt 2
 * @param count1 počítadlo, koľko hodnôt sa zobralo z prvej množiny 
 * @param count2 počítadlo, koľko hodnôt sa zobralo z druhej množiny 
 * @return maximálna hodnota z vrcholu zoznamov
 */
int getMax(std::deque<int>& deq1, std::deque<int>& deq2, int& count1, int& count2)
{
    auto elem1 = deq1.front();
    auto elem2 = deq2.front();

    if (deq1.size() == 0) // ak je jedna množina prázdna, vráti sa prvok z druhej množiny
    {
        count2--;
        return getFirst(deq2);
    }
    if (deq2.size() == 0) 
    {
        count1--;
        return getFirst(deq1);
    }
    if (elem1 >= elem2)
    {
        deq1.pop_front();
        count1--;
        return elem1;
    }
    else
    {
        deq2.pop_front();
        count2--;
        return elem2;
    }
    return 0;
}

/**
 * Vyberie sa z 2 množín väčšia hodnota a vymaže ju
 * @param deq1 množina hodnôt 1
 * @param deq2 množina hodnôt 2
 * @return maximálna hodnota z vrcholu zoznamov
 */
int getMax(std::deque<int>& deq1, std::deque<int>& deq2)
{
    auto elem1 = deq1.front();
    auto elem2 = deq2.front();

    if (deq1.size() == 0) // ak je jedna množina prázdna, vráti sa prvok z druhej množiny
        return getFirst(deq2);
    if (deq2.size() == 0) 
        return getFirst(deq1);
    if (elem1 >= elem2)
    {
        deq1.pop_front();
        return elem1;
    }
    else
    {
        deq2.pop_front();
        return elem2;
    }
    return 0;
}

/**
 * Funkcia, ktorú vykonáva každý proces, ktorý nieje prvý ani posledný
 * @param myId čislo aktuálneho procesu
**/
void MiddleProcesses(int myId)
{
    int recvValue; // obdržaná hodonta
    int sendValue; // číslo, ktoré sa pošle
    MPI_Status stat;
    std::deque<int> firstPipeline;
    std::deque<int> secondPipeline;
    
    int tag = 0; // určuje, do ktorej pipeliny sa uloži číslo
    int limitFirstPipeline = pow(2, myId-1); // limit prvej pipeliny na porovnávanie a posielanie hodnôt dalším procesom 
    bool start = false; // ak sa dosiahne limit na porovnávanie posielanie hodnôť dalej, nastaví sa táto hodnota na true  
    bool finishRecv = false; // ak sa pošlú všetky hodnoty, nastaví sa táto hodnota na true 

    int constant = pow(2, myId)/2; // určuje počet čísel, ktoré sa môžu maximálne zobrat z jednej pipeliny;
    int numberTakenFromFirst = constant; // počet čísel, ktoré sa môžu maximálne zobrat z prvej pipeliny
    int numberTakenFromSecond = constant; // počet čísel, ktoré sa môžu maximálne zobrat z druhej pipeliny

    while (true)
    {
        if (finishRecv == false)
        {
            MPI_Recv(&recvValue, 1, MPI_INT, myId-1, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            switch (stat.MPI_TAG)
            {
                case TAG_FIRST_PIPELINE: firstPipeline.push_back(recvValue); break;
                case TAG_SECOND_PIPELINE: secondPipeline.push_back(recvValue); break;
                case TAG_END: finishRecv = true; break;
                default: break;
            }
        }
        if ( (firstPipeline.size() == limitFirstPipeline) && (secondPipeline.size() == 1)) // podmienka na začiatok radenia, ak je splená može sa začať radiť
            start = true;

        if (start == true)
        {
            //vyberá sa výstupná hodnota sendValue
            //----------
            if (numberTakenFromFirst == 0) 
            {
                sendValue = getFirst(secondPipeline);
                numberTakenFromSecond--;
            }
            else if (numberTakenFromSecond == 0)
            {
                sendValue = getFirst(firstPipeline);
                numberTakenFromFirst--;
            }
            else
                sendValue = getMax(firstPipeline,secondPipeline, numberTakenFromFirst,numberTakenFromSecond);
            //----------
            MPI_Send(&sendValue, 1, MPI_INT, myId+1, tag, MPI_COMM_WORLD);
            
            if ((numberTakenFromFirst == 0) && (numberTakenFromSecond == 0)) //začína další cyklus, počet cyklov je 2^(počet procesov -1)
            {
                numberTakenFromFirst = constant;
                numberTakenFromSecond = constant;
                tag = (tag+1) % 2; // zvyšuje tag
            }
            
            if ((finishRecv == true) && (firstPipeline.size() == 0) && (secondPipeline.size() == 0)) // ak process už nemá čo robiť tak skončí
            {
                MPI_Send(&sendValue, 1, MPI_INT, myId+1, TAG_END, MPI_COMM_WORLD); // posiela tag nasledujúcemu procesu, že už skončil
                return;
            }
        }
    }
}

/**
 * Túto funkciu vykonáva posledný procesor
 * @param myId id posledného procesora 
 */
void LastProcess(int myId)
{
    int recvValue; // obdržaná hodonta
    MPI_Status stat;
    std::deque<int> firstPipeline;
    std::deque<int> secondPipeline;
    std::deque<int> Pipeline; // výsledná množina v ktorej sa bude nachadzať zoradená postupnosť
    bool start = false; 
    bool finishRecv = false;
    int limitFirstPipeline = pow(2, myId-1);
    while (true)
    {
        if (finishRecv == false)
        {
            MPI_Recv(&recvValue, 1, MPI_INT, myId-1, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            switch (stat.MPI_TAG)
            {
                case TAG_FIRST_PIPELINE: firstPipeline.push_back(recvValue); break;
                case TAG_SECOND_PIPELINE: secondPipeline.push_back(recvValue); break;
                case TAG_END: finishRecv = true; break;
                default: break;
            }
        }

        if ( (firstPipeline.size() == limitFirstPipeline) && (secondPipeline.size() == 1)) // podmienka na začiatok radenia
            start = true;

        if (start == true)
        {
            int val = getMax(firstPipeline,secondPipeline); //vyberá sa najväčšia hodnota
            Pipeline.push_back(val);
            if ((finishRecv == true) && (firstPipeline.size() == 0) && (secondPipeline.size() == 0)) // ked sa priali všetky hodonty
            {
                printAtTheEnd(Pipeline);
                return;
            }
        }
    }
}

/**
 * Prvý procesor posiela právy inému procesoru
 * @param to id procesora, ktorému sa budú posielať správy
 * @param to množina hodnôt. ktorá sa má poslať
 */
void sendAll(int to, const std::deque<int>& values)
{
    int tag = 0;
    int item = 0; 
    for (auto elem = values.begin(); elem != values.end(); elem++)
    {
        item = (int)*elem;
        MPI_Send(&item, 1, MPI_INT, to, tag, MPI_COMM_WORLD);
        tag = (tag+1) % 2;
    }
    MPI_Send(&item, 1, MPI_INT, to, TAG_END, MPI_COMM_WORLD);
}

int main(int argc, char *argv[])
{
    int processMax;
    int processID; //aktuálne id procesora
    std::deque<int> numbers; //všetky generované čísla
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD,&processMax);
    MPI_Comm_rank(MPI_COMM_WORLD,&processID);
    if (processID == 0) // hlavný proces
    {  
        std::string FileName = "numbers"; //meno súbora z číslami
        std::ifstream input( FileName , std::ios::binary );
        if (input.is_open() == false) // kontrola či sa dá otvoriť
        {
            std::cerr << "Input file is bad" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        while (input.good()) //naplní vektor vstupnými hodontami
        {
            int number = input.get();
            if (!input.good())
                break;
            numbers.push_back(number);
        }

        printVector(numbers);
        
        if (processMax == 1) // toto je v prípade ak je iba jeden procesor, tak sa rovno vypíše vstup
        {
            printVector(numbers);
            MPI_Finalize();
            return 0;
        }
        sendAll(processID+1,numbers); // pošle všetky vstupné hodnoty dalšiemu procesoru
    }

    if ((processID > 0) && (processID < (processMax-1) ))
    {
        MiddleProcesses(processID);
    } 
    
    if (processID == (processMax-1))
    {
        LastProcess(processID);
    }

    MPI_Finalize();
    return 0;
}
