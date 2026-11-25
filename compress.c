#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

struct Bitmap {
    int rows;
    int cols;
    short *pixels;
};

struct Node {
    short intensity;
    struct Node *children[4];
};

struct Tree {
    struct Node *root;
};

struct Bitmap Bitmap_new(char *fname) {
    struct Bitmap bitmap;
    FILE *file;
    int num,i;
    char line[5];
    char *word;
    bitmap.rows = 0;
    /*Count number of lines in file*/
    file = fopen(fname, "r");
    while (fgets(line, sizeof(line), file) != NULL) {
        word = strtok(line, " \t\n");
        while (word != NULL) {
            bitmap.rows++;
            word = strtok(NULL, " \t\n");
        }
    }
    fclose(file);
    bitmap.rows = sqrt(bitmap.rows);
    bitmap.cols = bitmap.rows;

    bitmap.pixels = malloc(bitmap.rows * bitmap.cols * sizeof(short));
    file = fopen(fname, "r");
    i = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        word = strtok(line, " \t\n");
        while (word != NULL) {
            num = atoi(word);
            bitmap.pixels[i] = (short)num;
            i++;
            word = strtok(NULL, " \t\n");
        }
    }
    fclose(file);
    return bitmap;
}

void Bitmap_clear(struct Bitmap *b) {
    b->cols = 0;
    b->rows = 0;
    free(b->pixels);
    b->pixels = NULL;
}

void Bitmap_rotate_left(struct Bitmap *b) {
    int i,j,last,currentCol;
    int size = b->rows*b->cols;
    short *temp = malloc(size * sizeof(short));
    /*create copy of pixels*/
    for(i=0;i<size;i++){
        temp[i] = b->pixels[i];
    }
    currentCol = b->cols-1;
    last = b->cols-1;
    /*replace each row with the right most column*/
    for(i=0;i<b->rows;i++){
        for(j=0;j<b->cols;j++){
            b->pixels[j+i*b->rows] = temp[last];
            last+=b->cols;
        }
        currentCol--;
        last = currentCol;
    }
    free(temp);
    temp = NULL;
}

void Bitmap_rotate_right(struct Bitmap *b) {
    int i,j,last,currentCol;
    int size = b->rows*b->cols;
    short *temp = malloc(size * sizeof(short));
    /*create copy of pixels*/
    for(i=0;i<size;i++){
        temp[i] = b->pixels[i];
    }
    currentCol = b->cols-1;
    last = b->cols-1;
    /*replace right most column with each row*/
    for(i=0;i<b->rows;i++){
        for(j=0;j<b->cols;j++){
            b->pixels[last] = temp[j+i*b->rows];
            last+=b->cols;
        }
        currentCol--;
        last = currentCol;
    }
    free(temp);
    temp = NULL;
}

void Bitmap_flip_vertical(struct Bitmap *b) {
    int i,temp,j;
    int bottom = b->cols*(b->cols-1);
    for(i=0;i<(b->rows/2);i++){
        for(j=0;j<b->cols;j++){
            temp = b->pixels[j+i*b->cols];
            b->pixels[j+i*b->cols] = b->pixels[bottom];
            b->pixels[bottom] = temp;
            bottom++;
        }
        bottom-=b->rows*2;
    }
}

void Bitmap_flip_horizontal(struct Bitmap *b) {
    int i,temp,left,right;
    int j = 0;
    /*go down the rows*/
    for(i=0;i<b->rows;i++){
        left = j;
        right = (b->cols-1)+j;
        /*reverse each row*/
        while(left<right){
            temp = b->pixels[left];
            b->pixels[left] = b->pixels[right];
            b->pixels[right] = temp;
            left++;
            right--;
        }
        /*move right pointer to last item in next row*/
        j+=b->cols;
    }
}

int allEqual(struct Bitmap *b, int x, int y, int cols) {
    int i,j,intensity;
    intensity = b->pixels[y * b->cols + x];
    for(i=y;i<cols+y;i++){
        for(j=x;j<cols+x;j++){
            if(b->pixels[i*b->cols + j] != intensity)
                return 0;
        }
    }
    return 1;
}

struct Node* build_tree(struct Bitmap *b, int x, int y,int cols) {
    int i;
    struct Node *node = malloc(sizeof(struct Node));
    
    if (allEqual(b,x,y,cols)) {
        node->intensity = b->pixels[y * b->cols + x];
        for (i = 0; i < 4; i++) {
            node->children[i] = NULL;
        }
        return node;
    }
    node->intensity = -1;
    cols /= 2;
    node->children[0] = build_tree(b, x, y, cols);                  
    node->children[1] = build_tree(b, x + cols, y, cols);     
    node->children[2] = build_tree(b, x, y + cols, cols);      
    node->children[3] = build_tree(b, x + cols, y + cols, cols);  
    return node;
}

struct Tree Tree_new(struct Bitmap *b) {
    struct Tree tree;
    tree.root = build_tree(b, 0, 0,b->cols);
    return tree;
}

void free_nodes(struct Node *node) {
    int i;
    if (node == NULL) return;
    for (i = 0; i < 4; i++) {
        free_nodes(node->children[i]);
    }
    free(node);
}

void Tree_clear(struct Tree *t) {
    free_nodes(t->root);
    t->root = NULL;
}

void print_node(struct Node *node, FILE *file) {
    int i;
    if (node == NULL) return;

    fprintf(file, "%d\n", node->intensity);

    for (i = 0; i < 4; i++) {
        print_node(node->children[i], file);
    }
}

void Tree_save(struct Tree *t, char *fname) {
    FILE *file = fopen(fname, "w");
    print_node(t->root, file);
    fclose(file);
}

void printMap(struct Bitmap *b){
    int i;
    int j=0;
    int size = b->rows*b->cols;
    for(i=0;i<size;i++){
        if(j==b->rows){
            printf("\n");
            j=0;
        }
        j++;
        printf("%d ",b->pixels[i]);
    }
    printf("\n\n");

}

int main(int argc, char *argv[]) {
    struct Tree tree;
    char *fname = calloc(strlen(argv[1]) + 12, sizeof(char));
    int choice;
    struct Bitmap bitmap = Bitmap_new(argv[1]);
    printMap(&bitmap);

    printf("Options menu:\n");
    printf("1. Rotate Left (counterclockwise)\n");
    printf("2. Rotate Right (clockwise)\n");
    printf("3. Flip Vertical\n");
    printf("4. Flip Horizontal\n");
    printf("5. Save and Quit\n");
    strcat(fname,argv[1]);
    strcat(fname,".compressed");

    do{
        printf("Your choice? ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                Bitmap_rotate_left(&bitmap);
                break;
            case 2:
                Bitmap_rotate_right(&bitmap);
                break;
            case 3:
                Bitmap_flip_vertical(&bitmap);
                break;
            case 4:
                Bitmap_flip_horizontal(&bitmap);
                break;
            case 5:
                tree = Tree_new(&bitmap);
                Tree_save(&tree,fname);
                break;
        }
    }while(choice != 5);
    printMap(&bitmap);
    free(fname);
    Bitmap_clear(&bitmap);
    Tree_clear(&tree);
    argc = argc;
    return 0;
}

/*gcc -ansi -Wall -Wextra -Wpedantic -Werror compress.c -lm*/