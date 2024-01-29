#include <stdlib.h>
#include <stdio.h>
#include "main.h"


typedef unsigned char byte;
typedef int error_code;

#define ERROR (-1)
#define HAS_ERROR(code) ((code) < 0)
#define HAS_NO_ERROR(code) ((code) >= 0)

/**
 * Cette fonction compare deux chaînes de caractères.       
 * @param p1 la première chaîne
 * @param p2 la deuxième chaîne
 * @return le résultat de la comparaison entre p1 et p2. Un nombre plus petit que
 * 0 dénote que la première chaîne est lexicographiquement inférieure à la deuxième.
 * Une valeur nulle indique que les deux chaînes sont égales tandis qu'une valeur positive
 * indique que la première chaîne est lexicographiquement supérieure à la deuxième.
 */
int strcmp(char *p1, char *p2) {
    char *s1 = (char *) p1;
    char *s2 = (char *) p2;
    char c1, c2;
    do {
        c1 = (char) *s1++;
        c2 = (char) *s2++;
        if (c1 == '\0')
            return c1 - c2;
    } while (c1 == c2);
    return c1 - c2;
}

/**
 * Ex. 1: Calcul la longueur de la chaîne passée en paramètre selon
 * la spécification de la fonction strlen standard
 * @param s un pointeur vers le premier caractère de la chaîne
 * @return le nombre de caractères dans le code d'erreur, ou une erreur
 * si l'entrée est incorrecte
 */
error_code strlen2(char *s) {
    if (s == NULL){
        return ERROR;
    }
    int count = 0;
    while(*s != '\0' && *s != '\n'){
        count++;
        s++;                                                // move on to the next character
    };
    return count;
}


/**
 * Ex.2 :Retourne le nombre de lignes d'un fichier sans changer la position
 * courante dans le fichier.
 * @param fp un pointeur vers le descripteur de fichier
 * @return le nombre de lignes, ou -1 si une erreur s'est produite
 */
error_code no_of_lines(FILE *fp) {
    if(fp == NULL){
        return ERROR;
    }
    int countLine = 0;
    long fpIndicator = ftell(fp);             //where the file position indicator is now
    int nextChar;
    while ((nextChar = fgetc(fp)) != EOF){
        if (nextChar == '\n'){
            countLine++;
        }
    }
    if(countLine!=0){countLine++;}

    fseek(fp, fpIndicator, SEEK_SET);         //sets the file position indicator to where it originally was
    return countLine;
}

/**
 * Ex.3: Lit une ligne au complet d'un fichier
 * @param fp le pointeur vers la ligne de fichier
 * @param out le pointeur vers la sortie
 * @param max_len la longueur maximale de la ligne à lire
 * @return le nombre de caractère ou ERROR si une erreur est survenue
 */
error_code readline(FILE *fp, char **out, size_t max_len) {
    if(fp == NULL){return ERROR;}

    // an extra byte is allocated for the null character '\0' which indicates the end of a string
    char *x = malloc((max_len+1) * sizeof(char));          //returns a pointer
    int count = 0; int nextChar;

    while ((nextChar = fgetc(fp)) != EOF){
        if(nextChar == '\n'){break;}                 //cursor moves to the next line in case don't fclose
        x[count] = (char) nextChar;                  //cast int to char
        count++;
    }
    x[count] = '\0';
    *out = x;

    return count;

}

/**
 * Ex.4: Copie un bloc mémoire vers un autre
 * @param dest la destination de la copie
 * @param src  la source de la copie
 * @param len la longueur (en byte) de la source
 * @return nombre de bytes copiés ou une erreur s'il y a lieu
 */
error_code memcpy2(void *dest, void *src, size_t len) {
    if(dest == NULL || src == NULL){
        return ERROR;
    }
    for (int i = 0; i < len; i++) {
        ((byte *) dest)[i] = ((byte *) src)[i];             //copy byte by byte
    }
    return len;
}

/**
 * Ex.5: Analyse une ligne de transition
 * @param line la ligne à lire
 * @param len la longueur de la ligne
 * @return la transition ou NULL en cas d'erreur
 */
transition *parse_line(char *line, size_t len) {
    transition *t = (transition *) malloc(sizeof(transition));
    if (t == NULL) { return NULL; }

    int linePos = 0;                                                      //position 0 of the string
    int currStateLen = 0;
    int nextStateLen = 0;

    for (int i = 1; i < len; i++) {                                       //starting from the first char, avoid '('
        if (line[i] == ',') {break;}
        currStateLen++;
    }
    t->current_state = (char *) malloc(sizeof(char) * (currStateLen + 1));  // +1 for '\0'
    if (t->current_state == NULL) {
        free(t->current_state);
        return NULL;
    }

    memcpy2(t->current_state, &line[1], currStateLen);                  //copy current state
    (t->current_state)[currStateLen] = '\0';

    linePos += currStateLen + 2;                                        //move to the symbol to read
    t->read = line[linePos];
    linePos += 5;                                                       //move to the next state

    for (int j = linePos; j < len; j++) {
        if (line[j] == ',') {break;}
        nextStateLen++;
    }

    t->next_state = (char *) malloc(sizeof(char) * (nextStateLen + 1));  // +1 for '\0'
    if (t->next_state == NULL) {
        free(t->next_state);
        return NULL;
    }

    memcpy2(t->next_state, &line[linePos], nextStateLen);                     //allocate memory for next state
    (t->next_state)[nextStateLen] = '\0';

    linePos += currStateLen + 1;                                        //move to the symbol to write
    t->write = line[linePos];
    linePos += 2;                                                       //move to the movement

    switch (line[linePos]) {
        case 'G': t->movement = -1; break;                                     //int????
        case 'D': t->movement = 1; break;
        default: t->movement = 0; break;
    }

    return t;
}


/**
 * Ex.6: Execute la machine de turing dont la description est fournie
 * @param machine_file le fichier de la description
 * @param input la chaîne d'entrée de la machine de turing
 * @return le code d'erreur
 */
error_code execute(char *machine_file, char *input) {
    int finalState;
    char *current;
    char *accept;
    char *refuse;

    FILE *fp = fopen(machine_file, "r");
    if (fp == NULL) {
        return ERROR;
    }

    int whichLine = 0;                                                    //at which line fo the doc
    char **cursor = (char **) malloc(sizeof(char *));                    //position to read

    while (whichLine < 3) {                                               // read 3 initial states
        int stateLen = readline(fp, cursor, 6);                           //5(max)+1(for '\0')
        switch (whichLine) {
            case 0: {
                current = (char *) malloc(sizeof(char) * (stateLen + 1));
                memcpy2(current, *cursor, stateLen);
                current[stateLen] = '\0';
                free(*cursor);
                break;
            }
            case 1: {
                accept = (char *) malloc(sizeof(char) * (stateLen + 1));
                memcpy2(accept, *cursor, stateLen);
                accept[stateLen + 1] = '\0';
                free(*cursor);
                break;
            }
            case 2 : {
                refuse = (char *) malloc(sizeof(char) * (stateLen + 1));
                memcpy2(refuse, *cursor, stateLen);
                refuse[stateLen] = '\0';
                free(*cursor);
                break;
            }
        }
        whichLine++;
    }

    whichLine = 0;                                                           //start from the 1st line of descriptions
    int descripLines = no_of_lines(fp);
    transition *t = (transition *) malloc(sizeof(transition) * descripLines);  //table that contains all descriptions
    while (whichLine < descripLines) {
        readline(fp, cursor, 23);                                             //max length of description Line--23
        t[whichLine] = *parse_line(*cursor, strlen2(*cursor));
        free(*cursor);
        whichLine++;
    }

    int ribLen = 2 * strlen2(input) + 1;
    char *ribbon = (char *) malloc(sizeof(char) * ribLen);                    //malloc for a working ribbon
    memcpy2(ribbon, input, strlen2(input));                                   //copy the input word tp ribbon
    for(int space = strlen2(input); space<ribLen; space++){
        ribbon[space] = ' ';                                                  //fill the rest of ribbon with ' '
    }

    int head = 0;                                                             //head position


    while (strcmp(current, accept) != 0 && strcmp(current, refuse) != 0) {    //reach the final state?

        if (finalState == -1) { break; }                                      //finalState set, exit 'while'


        for (int i = 0; i <= descripLines; i++) {
            //no state match
            if (i == descripLines) {
                finalState = -1;
                break;
            }

            transition description = t[i];
            //find the matched description and change the ribbon
            if (strcmp(description.current_state, current) == 0 && description.read == ribbon[head]) {
                ribbon[head] = description.write;
                memcpy2(current, description.next_state, strlen2(description.next_state) + 1);
                head += description.movement;                                 //move to next char of the ribbon
                // next round of match search
                break;
            }

            if (head == (ribLen - 1)) {                                       //need more space in ribbon
                ribLen += strlen2(input);
                char *longerRib = (char *) malloc(sizeof(char) * ribLen);
                memcpy2(longerRib, ribbon, head);
                for(int space = strlen2(input); space<ribLen; space++){
                    ribbon[space] = ' ';                                      //fill the rest of ribbon with ' '
                }

                free(ribbon);
                ribbon = longerRib;                                           //longer working ribbon generated
            }

        }
    }

    if (strcmp(current, accept) == 0) { finalState = 1; }
    else if (strcmp(current, refuse) == 0) { finalState = 0; }


    free(ribbon); free(accept); free(refuse); free(current);

    for(int j = 0; j < descripLines; j++){                                 //free memory of the description table
        free(t[j].current_state);
        free(t[j].next_state);
    }

    free(t); free(cursor); fclose(fp);

    return finalState;
}


// ATTENTION! TOUT CE QUI EST ENTRE LES BALISES ༽つ۞﹏۞༼つ SERA ENLEVÉ! N'AJOUTEZ PAS D'AUTRES ༽つ۞﹏۞༼つ

// ༽つ۞﹏۞༼つ

int main() {
// Vous pouvez ajouter des tests pour les fonctions ici
    /**
    char str1[] = "hello";
    char str2[] = "world";
    int result1 = strcmp(str1, str2);
    printf("Comparison of str1 and str2: %d\n", result1);*/

    //char str[] = "Hello";                                           //exercice1
    //int length = strlen2(str);
    //printf("Length of the string: %d\n", strlen2("H"));




    //FILE *file = fopen("has_five_ones","r");                       //exercice2
    //printf("%d", no_of_lines(file));
    //fclose(file);



    /**FILE *file2 = fopen("simple.txt", "r");                    //exercice3
    char **out = malloc(sizeof(char *));
    printf("%d\n", readline(file2, out, 100));
    char **out1 = malloc(sizeof(char *));
    printf("%d\n", readline(file2, out1, 100));

    fclose(file2);
    free(*out1);
    free(out1);
    free(*out);
    free(out);
    */


    /**char *line = "(q0,0)->(qR,1,D)\n";                         //exercice5
    transition *t = parse_line(line, strlen2(line));
    printf("%c",*(t->current_state));
    (t->current_state)++;
    printf("%c\n",*(t->current_state));

    printf("%c",*(t->next_state));
    (t->next_state)++;
    printf("%c\n",*(t->next_state));

    printf("%d\n",t->movement);
    printf("%c\n",t->read);
    printf("%c\n",t->write);
    free(t->next_state);
    free(t->current_state);*/



    //printf("%d\n", execute("has_five_ones", "101010101"));             //exercice6
    //printf("%d\n", execute("has_five_ones", "H"));
    //printf("%d\n", execute("has_five_ones", "Hello"));
    //printf("%d\n", execute("has_five_ones", "10101010"));
    //printf("%d\n", execute("has_five_ones", "0111"));


    return 0;
}






// ༽つ۞﹏۞༼つ