/*
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your group information here.
 *
 * Group No.: 8 (Join a project group in Canvas)
 * First member's full name: Haotian Lu
 * First member's email address: haotianlu2-c@my.cityu.edu.hk
 * Second member's full name: Jiuheng Zhang
 * Second member's email address: jiuhzhang2-c@my.cityu.edu.hk
 * Third member's full name: (leave blank if none)
 * Third member's email address: (leave blank if none)
 */
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <stdbool.h>
#include <libgen.h>
#include <math.h>
#include <inttypes.h>
/* thread argument struct*/
struct ca{
  int index;
};

/* Global Variables Declaration */
char** address; // variable used to store the result from mmap()
struct stat* sb; //buffer to store stat structs from files
int* fds;  //an array to keep track of all the file descriptors
int** results; // an array to store results for each thread
pthread_mutex_t o; //mutex lock
int num_of_files=0;
int num_of_threads;//=get_nprocs();
struct ca* c;
char** file; // variable to store file names


/* function declarations */
void* count(void* arg); //func for pthread
void swap(char* a, char* b);
void output(int index);
void quick_sort(char** f,int first,int last);

/* main */
int main(int argc, char** argv){
  if(argc<2){
    printf("pzip: file1 [file2 ...]\n");
        return -1;
  }
/* variable declarations and memory allocation for reading the arguments */
  DIR * dirPos;     // variables used for accessing a directory
  struct dirent* entry;
  char* full_path=malloc(2000*sizeof(char));  //used to store path for files inside a directory
  file=malloc(1000*sizeof(char*));
        for(int i =0; i<1000;i++){
        file[i]=malloc(1000*sizeof(char));
    }

/* reading the files from the arguments and store their names*/
    for(int i=1;i<argc;i++){
        struct stat statbuf;
        stat(argv[i],&statbuf);
        if(S_ISDIR(statbuf.st_mode)){ //Open the directory and store the path and name of each file separatly.
            dirPos=opendir(argv[i]);
            strcpy(full_path,argv[i]);
        while ((entry = readdir(dirPos)) != NULL){
        if(strcmp(entry->d_name,".")!=0&&strcmp(entry->d_name,"..")!=0){
            char* cur=malloc(1000*sizeof(char));
            strcpy(cur,full_path);
            strcat(cur,"/");
            strcat(cur,entry->d_name);
            strcpy(file[num_of_files],cur);
            free(cur);
            num_of_files++;
        }

    }
    closedir(dirPos);
        }
        else{// the argument is not a dir, just a file
             strcpy(file[num_of_files],argv[i]);
             num_of_files++;
        }

    }
    free(full_path);
    /* Apply quick_sort to make the all file names in alphabetical order.*/
    quick_sort(file,0,num_of_files-1);

    /* memory allocation and variables init for mapping input files to memory */
    address=malloc(num_of_files*sizeof(char*));
    sb =malloc(num_of_files*sizeof(struct stat));
    fds=malloc(num_of_files*sizeof(int));
    results=malloc(num_of_files*sizeof(int*));
    for(int i=0;i<num_of_files;i++){
      results[i]=malloc(26*sizeof(int));
      for(int j =0;j<26;j++){
        results[i][j]=0;
      }
    }




    /* mapping files to memory for easy and fast access */
    for(int i =0;i<num_of_files;i++){
        fds[i]=open(file[i],O_RDONLY);
        fstat(fds[i],&sb[i]);
        address[i]=mmap(NULL,sb[i].st_size,PROT_READ,MAP_SHARED, fds[i],0);
    }
    /* Varibles declarations and init for threads operations*/
    c=malloc(num_of_files*sizeof(struct ca));
    pthread_t pid[num_of_files];
    pthread_mutex_init(&o, NULL);
    num_of_threads=get_nprocs();
    /* threads manipulation */
    int remaining_files=num_of_files;
    int index=0;


    while(remaining_files>0){
      if(remaining_files<num_of_threads){
        /* todo:
         create 2 data struct to store arguments ( count_arg and thread_arg)
         use count_arg[num_of_files] to declare instead of malloc
         finish thread creating in main
         finish thread_create func
         change count func

         */
         for(int k=0;k<remaining_files;k++){
         c[k+index].index=k+index;
          pthread_create(&pid[k+index],NULL,count,(void*)&c[k+index]);
        }

        for(int k=0;k<remaining_files;k++){
          pthread_join(pid[k+index],NULL);
        }
        remaining_files=0;
        index=remaining_files-1;
      }
      else{
        for(int k=0;k<num_of_threads;k++){
        c[k+index].index=k+index;
         pthread_create(&pid[k+index],NULL,count,(void*)&c[k+index]);
       }

       for(int k=0;k<num_of_threads;k++){
         pthread_join(pid[k+index],NULL);
       }
       remaining_files=-num_of_threads;
       index=+num_of_threads;




          }
      }


    /* output */
    for(int k =0;k<num_of_files;k++){
      char var='a';
      printf("%s\n",file[k]);
      for(int j=0;j<26;j++){
        if(results[k][j]!=0){
        printf("%c %d\n",var,results[k][j]);
          }
          var++;
        }
    }


    /* free memory */
    free(sb);
    free(fds);

    for(int i =0;i<num_of_files;i++){
        munmap(address[i],sb[i].st_size);
        free(results[i]);
    }
    free(results);
    free(address);
    free(c);
    for(int i =0; i<1000;i++){
    free(file[i]);
    }
    free(file);
    pthread_mutex_destroy(&o);



  return 0;
}



void* count(void* arg){
  struct ca* sub =(struct ca*)arg;
  int* r=malloc(26*sizeof(int));
  for(int i=0;i<26;i++){
    r[i]=0;
  }
  for(size_t i=0;i<sb[sub->index].st_size;i++){

    char tmp = address[sub->index][i];
    int up=(int)tmp -97;
    if(0<=up&&up<26){
      r[up]++;
    }

  }
    pthread_mutex_lock(&o);
    for(int i=0;i<26;i++){
      results[sub->index][i]=results[sub->index][i]+r[i];
    }
    pthread_mutex_unlock(&o);
    free(r);




  return NULL;
}


/* subroutine implementations */

void swap(char* x,char* y){
    char* sub = malloc(1000*sizeof(char));
     strcpy(sub,x);
     strcpy(x,y);
    strcpy(y,sub);
    free(sub);

}

void quick_sort(char** f,int first ,int last){
    int pivot, low, high, q;
    if (first >= last) return;
    pivot =first ;
    low = first + 1;
    high = last;

    while (low < high) {
        while (strcmp(file[low],file[pivot])<0 && low < last)
            low++;
        while (strcmp(file[high],file[pivot])>0 && high > first)
            high--;
        if (low < high)
            swap(file[low], file[high]);
    }
    if (strcmp(file[pivot],file[high])>0) swap(file[pivot], file[high]);
    q = high;
    quick_sort(file, first, q - 1);
   quick_sort(file, q + 1, last);
    return;
}
