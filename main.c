#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define HUMAN '+'
#define SPACE ' '
#define WALL '#'
#define DOOR ':'

int rows,cols;
int is_occupied(int x,int y, char *map) {
    if(map[y*cols+x]==WALL || map[y*cols+x]==HUMAN) {
        return 1;
    }
    return 0;
}
int human_move(int x, int y,char *map, int *human_x, int *human_y,int which, int humans) {
        int pos_x = human_x[which] + x;
        int pos_y = human_y[which] + y;

        if(map[pos_y*cols+pos_x] == DOOR) {
            for(int human_index=which; human_index<humans-1; human_index++) {
                human_x[human_index] = human_x[human_index + 1];
                human_y[human_index] = human_y[human_index + 1];
            }
            map[(pos_y-y)*cols+pos_x-x] = SPACE;
            return 1;
        }
        human_x[which] += x;
        human_y[which] += y;

        pos_x = human_x[which];
        pos_y = human_y[which];

        map[(pos_y-y)*cols+pos_x-x] = SPACE;
        map[pos_y*cols+pos_x]=HUMAN;


        return 0;
}

void print_map(char *map) {
    printf("\e[1;1H\e[2J");
    for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {
            printf("%c",map[i*cols+j]);
        }
        printf("\n");
    }
}

int main()
{
    FILE *fp;
    char ch;
    //Uważaj, żeby nie było na końcu pliku białych znaków, bo nie będzie działać
    //Otwórz plik z mapą
    if((fp = fopen("mapa.txt", "r")) == NULL) {
        printf("File is not available \n");
        return 1;
    }

    //Policz wymiary mapy
    cols=0;
    rows=1;
    while ((ch = fgetc(fp)) != '\n') {
        cols++;
    }
    //Sprawdź czy mapa jest prostokątna
    int cols_temp=0;
    while ((ch = fgetc(fp)) != EOF)
    {
        cols_temp++;
        if (ch == '\n') {
            if(cols_temp-1!=cols){
                printf("Mapa powinna być prostokątna. Kończę program.");
                return 2;
            }
            rows++;
            cols_temp=0;
        }
    }
    fseek(fp, 0, SEEK_SET);

    //utwórz tablice z odwzorowaniem mapy
    char* map = (char*)calloc(rows*cols, sizeof(char));
    //utworz 2 tablice ze wspolrzednymi wolnego miejsca
    int* free_x=(int*)calloc((cols-1)*(rows-1), sizeof(int));
    int* free_y=(int*)calloc((cols-1)*(rows-1), sizeof(int));
    int free_index=0;

    for(int i=0; i<rows; i++) {
        for(int j=0; j<cols; j++) {
            if((ch = fgetc(fp))=='\n') {
                //Pomiń znaki końca linii
                ch=fgetc(fp);
            }
            //Jeśli wolne miejsce, dodaj do tablic "free_x free_y"
            if(ch == ' '){
                free_x[free_index]=j;
                free_y[free_index]=i;
                free_index++;
            }
            map[i*cols+j]=ch;
        }
    }
    fclose(fp);

    //Dodaj kilku ludzi w losowych miejscach
    int humans=2147483647;
    while(humans>free_index) {
        printf("Ilu ludzi dać do symulacji? (maksymalnie %d)\n",free_index);
        scanf("%d",&humans);
    }
    srand(time(NULL));
    //stworz 2 tablice ze wspolrzednymi ludzi
    int* human_x=(int*)calloc(humans, sizeof(int));
    int* human_y=(int*)calloc(humans, sizeof(int));
    int human_index=0;

    //Pomocnicza tablica do uniknięcia powtórzeń
    int* repetition = (int*)calloc(humans, sizeof(int));

    while(human_index<humans) {
        int human_pos = rand() % free_index;
        int repetition_flag=0;
        for(int i=0; i<human_index; i++) {
            if(repetition[i]==human_pos) {
                //printf("%d - dalej\n",human_pos);
                repetition_flag=1;
                break;
            }
        }
        if(repetition_flag == 1) {
            //wylosowana została ta sama wolna pozycja, więc powtorz losowanie
            continue;
        }
        human_x[human_index]=free_x[human_pos];
        human_y[human_index]=free_y[human_pos];
        repetition[human_index]=human_pos;
        human_index++;
    }

    //umieść ludzi do tablicy z mapą
    for(int i=0; i<humans; i++) {
        map[human_y[i]*cols+human_x[i]]=HUMAN;
    }


    //niech ludzie spacerują
    for(int t=0; t<10; t++) {
        human_index=0;
        while(human_index<humans) {
            int move_x=rand()%3-1;
            int move_y=rand()%3-1;
            if(is_occupied(human_x[human_index]+move_x,human_y[human_index]+move_y,map)) {
                human_index++;
                continue;
            }
            if(human_move(move_x,move_y,map,human_x,human_y,human_index,humans)==1) {
                humans--;
                human_index--;
            }
            human_index++;
        }
        print_map(map);
        printf("Ludzi na mapie: %d\n",humans);
        sleep(1);
    }

    //Ewakuacja
    while(humans>0) {
        for(human_index=0; human_index<humans; human_index++) {
            int move_x,move_y;
            if(human_y[human_index]<=3) {
                move_y = 1;
            } else if(human_y[human_index]>=5) {
                move_y = -1;
            } else {
                move_y = 0;
                if(human_x[human_index]<cols/2){
                    move_x = -1;
                } else {
                    move_x = 1;
                }
                if(is_occupied(human_x[human_index]+move_x, human_y[human_index]+move_y, map)==0) {
                    printf("Porusz\n");
                    if(human_move(move_x,move_y,map,human_x,human_y,human_index,humans)==1) {
                        humans--;
                        human_index--;
                    }
                }
                continue;
            }
            move_x = rand()%3-1;
            if(is_occupied(human_x[human_index]+move_x, human_y[human_index]+move_y, map)==0) {

                if(human_move(move_x,move_y,map,human_x,human_y,human_index,humans)==1) {
                    humans--;
                    human_index--;
                }
            }
        }
        print_map(map);
        printf("EWAKUACJA!\nLudzi na mapie: %d\n",humans);
        sleep(1);
    }

    printf("Koniec programu.\n");
    return 0;
}
