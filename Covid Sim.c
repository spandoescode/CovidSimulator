#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct event event;

#define max_neigbours 10
#define population 10000
#define initial_infected_number 3
#define final_day 300
/*** GLOBAL VARIABLES ***/

int grid[population][population]; // 2 dimensional array to record neighbours of each node
int neighbours[population];       //array to keep track of number of neighbours of each node. It ensures no node exceeds makimum permitted neighbours
//array to keep track of status of each node i.e. susceptible, infected or recovered. 0 indicates susceptible, 1 indicates infected and 2 indicates recovered
int status[population];
int counts[3]; //array to track total number of infected, recovered and susceptible nodes

/*** STRUCTURES ***/
//struct event is used to represent an event in the algortihm. Variable 'type' specifies whether event is a transmission or a recovery.
//'day' indicated day on which the event occurs and patient indicates the node number concerned with the event.
struct event
{
    int type; //1 for transmit, 2 for recovery
    int day;
    int patient;
};

struct event queue[20 * population]; //priority queue for the events in the algorithm
int size = 0;                        //stores size of the priority queue at any given time

//standard swap method to switch indices of events in the queue
void swap(int i, int j)
{
    int t = queue[i].day;
    queue[i].day = queue[j].day;
    queue[j].day = t;
    t = queue[i].type;
    queue[i].type = queue[j].type;
    queue[j].type = t;
    t = queue[i].patient;
    queue[i].patient = queue[j].patient;
    queue[j].patient = t;
}

//standard method to insert into the queue with 'day' taking on the roll of a key
void insert(int type, int day, int patient_num)
{
    //adding new event to the end of the queue
    queue[size].day = day;
    queue[size].type = type;
    queue[size].patient = patient_num;
    size++;           //updating size of the queue
    int i = size - 1; //stores child index during the bubble up process
    while (i != 0)
    {
        if (queue[i].day < queue[(i - 1) / 2].day) //case where parent index has a later occuring event
        {
            swap(i, (i - 1) / 2); //swapping parent and child index events
            i = (i - 1) / 2;      //updating child index for next iteration
        }
        else
            break; //loop termination case
    }
}

//standard deletion method for a priority queue
void delete ()
{
    swap(0, size - 1); //swapping first and last indexes
    size--;            //decrementing size to signify deletion of the last element
    int k = 0;         //used to keep track of parent index
    if (size > 0)      //checking for an empty queue
    {
        while (2 * k + 1 < size) //checking for valid child in heap
        {
            if (2 * k + 2 >= size)
            {
                if (queue[k].day > queue[2 * k + 1].day) //condition for swap required in the bubble down process
                {
                    swap(k, 2 * k + 1);
                    k = 2 * k + 1; //updating parent index to child with which it was swapped
                }
                else
                {
                    break; //termination condition for loop
                }
            }
            else //case of two valid children in the heap
            {
                //determining child with higher priority
                int parent = 2 * k + 1;
                if (queue[parent].day > queue[parent + 1].day)
                    parent += 1;
                if (queue[k].day > queue[parent].day) //case for swap required in the bubble down process
                {
                    swap(k, parent);
                    k = parent; //updating parent index to the child which was swapped with it
                }
                else
                    break; //loop termination condition
            }
        }
    }
}

//method to find days required for node to transmit
int days_to_transmit()
{
    int x = 1;
    while (rand() % 2 != 0)
    {
        x++;
    }
    return x;
}

//method to find days required for the node to recover
int days_to_recover()
{
    int x = 1;
    while (rand() % 5 != 0)
    {
        x++;
    }
    return x;
}

//method to process the earliest occuring event in the queue
void event_process(FILE *fptr)
{
    static int day = 1;      //variable to keep track of printing the numbers at regular intervals of five days
    if (queue[0].day >= day) //case where day threshold is met and numbers are to be printed
    {
        fprintf(fptr, "Day %d: S:%d I:%d R:%d\n", day, counts[0], counts[1], counts[2]);
        day += 1;
    }
    if (queue[0].type == 2) //case of recovery event
    {
        status[queue[0].patient] = 2; //updating node status to recovered
        counts[1]--;                  //decrementing infected total
        counts[2]++;                  //incrementing recovered total
    }
    else //case of a transmission event
    {
        if (status[queue[0].patient] == 0) //checking if recipient node is susceptible
        {
            status[queue[0].patient] = 1;                             //updating status of concerned node
            counts[0]--;                                              //decrementing susceptible count
            counts[1]++;                                              //incrementing infected count
            int recovery = days_to_recover();                         // calculating days required for recovery
            if (queue[0].day + recovery <= final_day)                 //clause to ensure only events upto day 300 are considered
                insert(2, queue[0].day + recovery, queue[0].patient); //pushing recovery event for node into the queue
            int infector = queue[0].patient;
            for (int i = 0; i < population; i++) //looping through all neighbours of node
            {
                if (grid[infector][i] == 1)
                {
                    if (status[i] == 0) //checkingn if neighbour is susceptible
                    {
                        int transmit = days_to_transmit();                               //calculating expected transmission day
                        if (transmit < recovery && queue[0].day + transmit <= final_day) //if condition to check for possible pseudo transmission after recovery
                        {
                            insert(1, queue[0].day + transmit, i);
                        }
                    }
                }
            }
        }
    }
    delete (); //removing processed event from the queue
}

//method to generate randomized graph of neighbours of nodes in the population
void generate()
{
    for (int i = 0; i < population; i++)
    {
        neighbours[i] = 0; //setting all neighbour counts to 0 initially
    }
    for (int i = 0; i < population; i++)
    {
        for (int j = 0; j < population; j++)
        {
            grid[i][j] = -1; //default placeholder value
            if (i == j)
                grid[i][j] = 0; //a node cannot be its own neighbour in this scenaerio
        }
    }
    for (int i = 0; i < population; i++)
    {
        for (int j = 0; j < population; j++)
        {
            if (i != j) //avoiding self neighbour case
            {
                //checking that particular position has not been determined before hand and concerned nodes haven't reached max neighnour limit
                if ((neighbours[i] < max_neigbours && neighbours[j] < max_neigbours) && grid[i][j] == -1)
                {
                    int res = rand() % 2;
                    //updating relation between node i and node j
                    grid[i][j] = res;
                    grid[j][i] = res;
                    neighbours[i] += res;
                    neighbours[j] += res;
                }
            }
        }
    }
    for (int i = 0; i < population; i++)
    {
        for (int j = 0; j < population; j++)
        {
            if (grid[i][j] == -1)
                grid[i][j] = 0; //removing default values and replacing with 'not neighbour' value
        }
    }
    for (int i = 0; i < population; i++)
    {
        status[i] = 0; //setting status of all nodes to initially susceptible
    }
    //setting counts to initial values
    for (int i = 0; i < 3; i++)
    {
        counts[i] = 0;
    }
    counts[0] = population;
}

int main(void)
{
    srand(time(NULL)); //seeding random number generator
    generate();        //generating graph of neighbours
    FILE *fptr_out;
    fptr_out = fopen("output.txt", "w"); //opening pointer to the output file
    //selecting three random nodes and infecting
    for (int i = 0; i < initial_infected_number; i++)
    {
        int p_zero = rand() % 10000;
        insert(1, 1, p_zero);
    }
    while (size != 0)
    {
        event_process(fptr_out); //processing events while queue in non empty
    }
    fprintf(fptr_out, "Exit numbers: %d %d %d", counts[0], counts[1], counts[2]); //printing final state of the queue at the end of day 300.
}
