#include <mqoai.h>
#include "C:\Users\rezac\CLionProjects\AssemblerRezaCh\cmake-build-debug\assemble.h"
#define Rtype 0
#define Itype 1
#define Jtype 2


int findSymTabLen(FILE *inputFile){
    int count=0;
    size_t lineSize;
    char *line;
    line=(char *)malloc(72);
    while(fgets(line,lineSize,inputFile) != NULL){
        if((line[0] == ' ') || (line[0] == '\t'));
        else
            count++;
    }
    rewind(inputFile);
    free(line);
    return count;
}
int fillSymTab(struct symbolTable *symT,FILE *inputFile){
    int lineNo = 0;
    int lineSize = 72;
    char *line;
    int i=0;
    char *token;
    line=(char *)malloc(72);
    while(fgets(line,lineSize,inputFile) != NULL){
        if((line[0] == ' ') || (line[0] == '\t'));
        else{
            token=strtok(line,"\t, ");
            strcpy(symT[i].symbol,token);
            symT[i].value=lineNo;
            i++;
        }
        lineNo++;
    }
    if(lineNo > 65536){
        printf("ERROR! overflow, size of memory is 65536 words!");
    }
    rewind(inputFile);
    free(line);
    return lineNo;
}
int hex2int( char* hex)
{
    int result=0;
    while ((*hex)!='\0')
    {
        if (('0'<=(*hex))&&((*hex)<='9'))
            result = result*16 + (*hex) -'0';
        else if (('a'<=(*hex))&&((*hex)<='f'))
            result = result*16 + (*hex) -'a'+10;
        else if (('A'<=(*hex))&&((*hex)<='F'))
            result = result*16 + (*hex) -'A'+10;
        hex++;
    }
    return(result);
}
void int2hex16(char *lower,int a){
    sprintf(lower,"%X",a);
    if(a <0x10){
        lower[4]='\0';
        lower[3]=lower[0];
        lower[2]='0';
        lower[1]='0';
        lower[0]='0';
    }
    else if(a <0x100){
        lower[4]='\0';
        lower[3]=lower[1];
        lower[2]=lower[0];
        lower[1]='0';
        lower[0]='0';
    }
    else if(a <0x1000){
        lower[4]='\0';
        lower[3]=lower[2];
        lower[2]=lower[1];
        lower[1]=lower[0];
        lower[0]='0';
    }
}
void duplicatedLabel(struct symbolTable *symT,int symbolTabLength){
    Boolean duplicated=FALSE;
    for (int i = 0; i < symbolTabLength; ++i) {
        for (int j = i+1; j < symbolTabLength; ++j) {
            if(strcmp(symT[i].symbol,symT[j].symbol)==0){
                duplicated=TRUE;
                break;
            }
        }
    }
    if(duplicated){
        puts("ERROR! exit(1) : Duplicated Label\n");
        exit(1);
    }
}

int validOpCode(char *token2,char *instructions[]){
    int opcode=-1;
    for (int op = 0; op < 15; op++) {
        if (strcmp(instructions[op], token2) == 0){
            opcode=op;
            break;
        }
    }
    return opcode;
}
int findSymbol(int symbolTabLength, struct symbolTable *symT, char *token) {
    int count = -1;
    for (int i = 0; i < symbolTabLength; i++) {
        if (strcmp(symT[i].symbol, token) == 0) {
            count = i;
        }
    }
    return count;
}

void result(int op, struct instruction *currentInstruction, struct symbolTable *symT, char hex_table[16], char lower[5], char *token, int symbolTabLength, int programCounter){
    if (op >= 0 && op <= 4) {//R format instruction
        currentInstruction->instType = Rtype;
        currentInstruction->mnemonic = (char*) malloc(5);
        strcpy(currentInstruction->mnemonic, token);
        token = strtok(NULL, ",\t\n ");
        currentInstruction->rd = atoi(token);
        token = strtok(NULL, ",\t\n ");
        currentInstruction->rs = atoi(token);
        token = strtok(NULL, ",\t\n ");
        currentInstruction->rt = atoi(token);
        currentInstruction->PC = currentInstruction->PC + 1;
        currentInstruction->inst[0] = '0';
        currentInstruction->inst[1] = hex_table[op];
        currentInstruction->inst[2] = hex_table[currentInstruction->rs];
        currentInstruction->inst[3] = hex_table[currentInstruction->rt];
        currentInstruction->inst[4] = hex_table[currentInstruction->rd];
        currentInstruction->inst[5] = '0';
        currentInstruction->inst[6] = '0';
        currentInstruction->inst[7] = '0';
        currentInstruction->inst[8] = '\0';
        int res = hex2int(currentInstruction->inst);
        currentInstruction->intInst=res;
    } else if (op >= 5 && op <= 12) {//I format instruction
        currentInstruction->instType = Itype;
        currentInstruction->mnemonic = (char*) malloc(5);
        strcpy(currentInstruction->mnemonic, token);
        if (strcmp(currentInstruction->mnemonic, "jalr") == 0) {
            token = strtok(NULL, ",\t\n ");
            currentInstruction->rt = atoi(token);
            token = strtok(NULL, ",\t\n ");
            currentInstruction->rs = atoi(token);
            currentInstruction->imm = 0;
        } else if (strcmp(currentInstruction->mnemonic, "lui") == 0) {
            token = strtok(NULL, ",\t\n ");
            currentInstruction->rt = atoi(token);
            token = strtok(NULL, ",\t\n ");
            currentInstruction->imm = atoi(token);
            currentInstruction->rs = 0;
        } else if (strcmp(currentInstruction->mnemonic, "beq") == 0) {
            token = strtok(NULL, ",\t\n ");
            currentInstruction->rs = atoi(token);
            token = strtok(NULL, ",\t\n ");
            currentInstruction->rt = atoi(token);
            token = strtok(NULL, ",\t\n ");
            int count = findSymbol(symbolTabLength, symT, token);
            if (count != -1) {
                currentInstruction->imm = symT[count].value - (programCounter);
            } else if(isalpha((token[0]))){
                puts("ERROR! exit(1) : Undefined Label\n");
                exit(1);
            }else{
                long num= atol(token);
                if(num<65536){
                    num =num - (programCounter);
                    currentInstruction->imm =num;
                }else {
                    puts("ERROR! exit(1) : invalid Offset\n");
                    exit(1);//invalid offset
                }
            }
        }else{
            token = strtok(NULL, ",\t\n ");
            currentInstruction->rs = atoi(token);
            token = strtok(NULL, ",\t\n ");
            currentInstruction->rt = atoi(token);
            token = strtok(NULL, ",\t\n ");
            int count = findSymbol(symbolTabLength, symT, token);
            if (count != -1) {
                currentInstruction->imm = symT[count].value;
            } else if(isalpha((token[0]))){
                puts("ERROR! exit(1) : Undefined Label\n");
                exit(1);
            }else{
                long num= atol(token);
                if(num<65536 && num>-65536){
                    currentInstruction->imm = num;
                }else {
                    puts("ERROR! exit(1) : invalid Offset\n");
                    exit(1);//invalid offset
                }
            }
        }
        currentInstruction->PC = currentInstruction->PC + 1;
        currentInstruction->inst[0] = '0';
        currentInstruction->inst[1] = hex_table[op];
        currentInstruction->inst[2] = hex_table[currentInstruction->rs];
        currentInstruction->inst[3] = hex_table[currentInstruction->rt];
        int2hex16(lower, currentInstruction->imm);
        currentInstruction->inst[4] = lower[0];
        currentInstruction->inst[5] = lower[1];
        currentInstruction->inst[6] = lower[2];
        currentInstruction->inst[7] = lower[3];
        currentInstruction->inst[8] = '\0';
        int res = hex2int(currentInstruction->inst);
        currentInstruction->intInst=res;
    } else if (op == 13 || op == 14) {//J format
        currentInstruction->instType = Jtype;
        currentInstruction->mnemonic = (char*) malloc(5);
        strcpy(currentInstruction->mnemonic, token);
        if (strcmp(currentInstruction->mnemonic, "halt") == 0) {
            currentInstruction->imm = 0;
        } else {//j
            token = strtok(NULL, ",\t\n ");
            int count = findSymbol(symbolTabLength, symT, token);
            if (count != -1) {
                currentInstruction->imm = symT[count].value;
            } else if(isalpha((token[0]))){
                puts("ERROR! exit(1) : Undefined Label\n");
                exit(1);
            }else {
                long num= atol(token);
                if(num<65536 && num>-65536){
                    currentInstruction->imm = num;
                }else {
                    puts("ERROR! exit(1) : invalid Offset\n");
                    exit(1);//invalid offset
                }
            }
        }
        currentInstruction->PC = currentInstruction->PC + 1;
        currentInstruction->inst[0] = '0';
        currentInstruction->inst[1] = hex_table[op];
        currentInstruction->inst[2] = '0';
        currentInstruction->inst[3] = '0';
        int2hex16(lower, currentInstruction->imm);
        currentInstruction->inst[4] = lower[0];
        currentInstruction->inst[5] = lower[1];
        currentInstruction->inst[6] = lower[2];
        currentInstruction->inst[7] = lower[3];
        currentInstruction->inst[8] = '\0';
        int res = hex2int(currentInstruction->inst);
        currentInstruction->intInst=res;
    }

}


void main(int argc,char **argv){
    Boolean invalidOpCode=TRUE;

    FILE *assp,*machp,*fopen();
    struct symbolTable *symbolTable;
    int symbolTableLength;
    int i,j,found,noInsts;
//    struct instruction *currInst;
    struct instruction *currInst = (struct instruction *) malloc(sizeof(struct instruction));
    size_t lineSize;
    char *token;
    char *instructions[]={"add","sub","slt","or","nand",
                          "addi","slti","ori","lui","lw","sw","beq","jalr",
                          "j","halt"};
    int instCnt=0;
    char hexTable[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    char lower[5];
    i=0;
    j=0;
    currInst=(struct instruction *)malloc(sizeof(struct instruction));
    if(argc < 3){
        printf("***** Please run this program as follows:\n");
        printf("***** %s assprog.as machprog.m\n",argv[0]);
        printf("***** where assprog.as is your assembly program\n");
        printf("***** and machprog.m will be your machine code.\n");
        exit(1);
    }
    if((assp=fopen(argv[1],"r")) == NULL){
        printf("%s cannot be opened\n",argv[1]);
        exit(1);
    }
    if((machp=fopen(argv[2],"w+")) == NULL){
        printf("%s cannot be opened\n",argv[2]);
        exit(1);
    }
    /*************************************************
     * ********************************************* *
     * ********************************************* *
     here, you can place your code for the assembler
     * ********************************************* *
     * ********************************************* *
     *************************************************/
    printf("Reza Charrakh : 4003613018 \n");

    symbolTableLength = findSymTabLen(assp);
    symbolTable = (struct symbolTable* ) malloc(symbolTableLength * sizeof(struct symbolTable));
    for (int k = 0; k < symbolTableLength; ++k) {
        symbolTable[k].symbol = (char*) malloc(7);
    }

    fillSymTab(symbolTable, assp);

    for (int k = 0; k < symbolTableLength; ++k) {
        printf("%s:%d \n", symbolTable[k].symbol, symbolTable[k].value);
    }

    duplicatedLabel(symbolTable, symbolTableLength);


    char *line = (char *) malloc(72);
    int programCounter = 0;
    while (fgets(line, 600, assp) != NULL) {
        programCounter++;
        if (line[0] == ' ' || line[0] == '\t') {
            token = strtok(line, "\t,\n ");
            int op= validOpCode(token, instructions);
            if(op >= 0){
                result(op, currInst, symbolTable, hexTable, lower, token, symbolTableLength, programCounter);
                printf("%d\n",currInst->intInst);
                fprintf(machp,"%d\n",currInst->intInst);
            }else {
                puts("ERROR! exit(1) : Invalid Opcode\n");
                exit(1);
            }
        }else{
            Boolean isInst = FALSE;
            token = strtok(line, "\t ");
            token = strtok(NULL, "\t\n ");
            for (int op = 0; op < 15; op++) {
                if (strcmp(instructions[op], token) == 0) {
                    isInst = TRUE;
                    result(op, currInst, symbolTable, hexTable, lower, token, symbolTableLength, programCounter);
                    printf("%d\n",currInst->intInst);
                    fprintf(machp,"%d\n",currInst->intInst);
                }
            }
            if (isInst == FALSE){
                if(strcmp(token,".fill")==0){
                    token = strtok(NULL, "\t\n ");
                    int found2 = 0;
                    int count = 0;
                    for (int i = 0; i < symbolTableLength; i++) {
                        if (strcmp(symbolTable[i].symbol, token) == 0) {
                            found2 = 1;
                            count = i;
                        }
                    }
                    if (found2) {
                        currInst->imm = symbolTable[count].value;
                    } else if(isalpha(token[0])){
                        puts("ERROR! exit(1) : Undefined Label\n");
                        exit(1);
                    }else{
                        currInst->imm = atoi(token);
                    }
                    currInst->intInst=currInst->imm;
                    printf("%d\n",currInst->intInst);
                    fprintf(machp,"%d\n",currInst->intInst);
                } else{
                    puts("ERROR! exit(1) : Invalid Opcode\n");
                    exit(1);
                }
            }
        }
    }
    fclose(assp);
    fclose(machp);
}
