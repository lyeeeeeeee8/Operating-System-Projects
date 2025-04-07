#include <iostream>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <time.h>
using namespace std;

/* global variables */
int num_threads;
float time_wait;
vector<string> policies;
vector<int> priorities;
pthread_barrier_t barrier;

/* the structure to store thread */
struct Thread_info
{
    int id;
    string policy;
    int priority;
};

void parse_arguments(int argc, char *argv[])
{
    int opt;
    char *str_policy, *str_priority;
    while((opt = getopt(argc, argv, "t:n:s:p:")) != -1)
    {
        // printf("opt = %c\n", opt);
        // printf("optarg = %s\n", optarg);
        switch(opt)
        {
            case 'n':
                num_threads = atoi(optarg);
                break;
            case 't':
                time_wait = atof(optarg);
                break;
            case 's':
                str_policy = optarg;
                for(char *token=strtok(str_policy, ","); token!=NULL; token=strtok(NULL, ","))
                    policies.push_back(token);
                break;
            case 'p':
                str_priority = optarg;
                for(char *token=strtok(str_priority, ","); token!=NULL; token=strtok(NULL, ","))
                    priorities.push_back(atoi(token));
                break;
            default:
                cerr << "Usage: " << argv[0] << " -n <num_threads> -t <time_wait> -s <policies> -p <priorities>\n";
                exit(EXIT_FAILURE);
        }
    }   
}

void busy_wait(double time_wait) 
{
    struct timespec start_time, current_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    while (true) 
    {
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        double elapsed_time = (current_time.tv_sec - start_time.tv_sec) +
                              (current_time.tv_nsec - start_time.tv_nsec) / 1e9;
        if (elapsed_time >= time_wait) 
            break;
    }
}

void *thread_function(void *arg)
{
    Thread_info *info = (Thread_info *)arg;

    /* 1. Wait until all threads are ready */
    pthread_barrier_wait(&barrier);

    /* 2. Do the task */ 
    for(int i=0; i<3; i++)
    {
        printf("Thread %d is starting\n", info->id);
        busy_wait(time_wait);
    }

    /* 3. Exit the function  */
    pthread_exit(NULL);
}
//---------------------------------------------------------------------------------------

int main(int argc, char *argv[]) 
{
    /* 1. Parse program arguments */
    parse_arguments(argc, argv);
   
    /* 5. Start all threads at once */
    if (pthread_barrier_init(&barrier, NULL, num_threads) != 0) 
    {
        perror("pthread_barrier_init");
        return 1;
    }

    /* 2. Create <num_threads> worker threads */
    vector<pthread_t> threads(num_threads);
    vector<Thread_info> threads_infos(num_threads);
    for(int i=0; i<num_threads; i++)
    {
        threads_infos[i] = {i, policies[i], priorities[i]};
    }

    /* 3. Set CPU affinity */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    /* 4. Set the attributes to each thread */
    for(int i=0; i<num_threads; i++)
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        sched_param param;
        param.sched_priority = threads_infos[i].priority;
        if(threads_infos[i].policy == "FIFO")
        {
            pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
            pthread_attr_setschedparam(&attr, &param);
        }
        else if(threads_infos[i].policy == "NORMAL")
        {
            pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
        }
        if (pthread_create(&threads[i], &attr, thread_function, &threads_infos[i]) != 0) 
        {
            cerr << "Error: Unable to create thread " << i << "\n";
            pthread_attr_destroy(&attr);
            return 1;
        }
        pthread_attr_destroy(&attr);    

        //////////////////////////////////////////////////////
        // int policy_check;
        // sched_param param_check;
        // pthread_getschedparam(threads[i], &policy_check, &param_check);
        // printf("Thread %d: policy=%s, priority=%d\n", 
        //     threads_infos[i].id, 
        //     (policy_check == SCHED_FIFO) ? "FIFO" : "OTHER", 
        //     param_check.sched_priority);
        // printf("\n\n");
        //////////////////////////////////////////////////////
        
    }

    /* 6. Wait for all threads to finish  */ 
    for (int i = 0; i < num_threads; i++) 
    {
        pthread_join(threads[i], nullptr);
    }
    pthread_barrier_destroy(&barrier);
    
    return 0;
}

