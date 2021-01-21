#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "varmanager.c"
#include <unistd.h>

#define TRUE 1
#define FALSE 0

typedef enum {
	END, INTEGER
} types;

#define END -1
#define INTEGER 1
#define ID 2
#define OP 3
#define EQUALS 4
#define SEMI 5
#define VAR 6
#define STRING 7
#define PRINT 8
#define IF 9
#define LPAREN 10
#define RPAREN 11
#define LBRACK 12
#define RBRACK 13
#define COMPARE 14
#define COMMA 15
#define INRANGE 16
#define FOR 17
#define ELIF 18
#define ELSE 19
#define WHILE 20
#define FUNC 21
#define RETURN 22

int debug = 0;
FILE* output;

void lines();
void error();
void goBack();
void lookAhead();
void callFunc();

void save(char * reg) {
	fprintf(output, "\tpushq %s\n", reg);
}

void restore(char * reg) {
	fprintf(output, "\tpopq %s\n", reg);
}

int getFileLength(char * filename) {
	FILE * f = fopen (filename, "rb");
	fseek (f, 0, SEEK_END);
	return ftell (f);
}

char * readlines(char * filename) {
	char * buffer = 0;
	long length;
	FILE * f = fopen (filename, "rb");

	if (f) {
		fseek (f, 0, SEEK_END);
		length = ftell (f);
		fseek (f, 0, SEEK_SET);
		buffer = malloc (length);
	}
	if (buffer) {
		fread (buffer, 1, length, f);
	} else {
		fclose (f);
	}

	if (buffer) {
		buffer[length] = '\0';
		return buffer;
	} else {
		return 0;
	}
}

int i = 0;
char * data;

typedef struct {
  int type;
  char * lexeme;
  int length;
} token;

token* currentToken;

void nextToken() {
	
	// skip whitespace
	while (isspace(data[i])) {
		i++;
	}
	
	if (i >= strlen(data)) {
		currentToken->type = EOF;
		currentToken->lexeme = "";
		currentToken->length = 1;
		i++;
		return;
	}
	
	// number
	if (isdigit(data[i])) {
		
		int length = 1;
		while (isdigit(data[i+length])) {
			length++;
		}
		
		char number[length+1];
		memcpy(number, data+i, length);
		number[length] = '\0';
		
		currentToken->type = INTEGER;
		currentToken->lexeme = malloc(sizeof(currentToken->lexeme));
		memcpy(currentToken->lexeme, number, length+1);
		currentToken->length = length;
		
	} else if (data[i] == '"') {
		currentToken->type = STRING;
		
		i++;
		int start = i;
		int length = 0;
		while (data[i+length] != '"') {
			length++;
		}
		i++;
		
		currentToken->lexeme = malloc(sizeof(currentToken->lexeme));
		strncpy(currentToken->lexeme, data + start, length);
		currentToken->lexeme[length] = '\0';
		currentToken->lexeme[length+1] = '\0';
		currentToken->lexeme[length+2] = '\0';
		currentToken->length = length+2;
		
		printf("\n%s\n", currentToken->lexeme);
		i-= 2;
	}
	// alphanumeric
	else if (isalpha(data[i])) {
		int length = 1;
		while (isalnum(data[i+length])) {
			length++;
		}
		
		char name[length+1];
		memcpy(name, data+i, length);
		name[length] = '\0';
		
		if (strcmp(name, "var") == 0) {
			currentToken->type = VAR;
		} else if (strcmp(name, "print") == 0) {
			currentToken->type = PRINT;
		} else if (strcmp(name, "if") == 0) {
			currentToken->type = IF;
		} else if (strcmp(name, "inrange") == 0) {
			currentToken->type = INRANGE;
		} else if (strcmp(name, "for") == 0) {
			currentToken->type = FOR;
		} else if (strcmp(name, "elif") == 0) {
			currentToken->type = ELIF;
		} else if (strcmp(name, "else") ==  0) {
			currentToken->type = ELSE;
		} else if (strcmp(name, "while") == 0) {
			currentToken->type = WHILE;
		} else if (strcmp(name, "func") == 0) {
			currentToken->type = FUNC;
		} else if (strcmp(name, "return") == 0) {
			currentToken->type = RETURN;
		} else {
			currentToken->type = ID;
		}
		currentToken->lexeme = malloc(sizeof(currentToken->lexeme));
		memcpy(currentToken->lexeme, name, length+1);
		currentToken->length = length;
		
	}
	// ops
	else if (data[i] == '+') {
		currentToken->type = OP;
		currentToken->lexeme = "+";
		currentToken->length = 1;
	} else if (data[i] == '-') {
		if (currentToken->type == INTEGER || currentToken->type == ID) {
			currentToken->type = OP;
			currentToken->lexeme = "-";
			currentToken->length = 1;
		} else {
			printf("Not an int");
			/* TODO: Account for negative sign */
			
			
			
			int length = 1;
			while (isdigit(data[i+length])) {
				length++;
			}
			
			char number[length+1];
			memcpy(number, data+i, length);
			number[length] = '\0';
			
			currentToken->type = INTEGER;
			currentToken->lexeme = malloc(sizeof(currentToken->lexeme));
			memcpy(currentToken->lexeme, number, length+1);
			currentToken->length = length;
		}
	} else if (data[i] == '*') {
		currentToken->type = OP;
		currentToken->lexeme = "*";
		currentToken->length = 1;
	} else if (data[i] == '/') {
		currentToken->type = OP;
		currentToken->lexeme = "/";
		currentToken->length = 1;
	}
	// comparison ops
	else if (data[i] == '=' && data[i+1] == '=') {
		currentToken->type = COMPARE;
		currentToken->lexeme = "==";
		currentToken->length = 2;
	}
	// equals
	else if (data[i] == '=') {
		currentToken->type = EQUALS;
		currentToken->lexeme = "=";
		currentToken->length = 1;
	}
	// semicolon
	else if (data[i] == ';') {
		currentToken->type = SEMI;
		currentToken->lexeme = ";";
		currentToken->length = 1;
	}
	// parens
	else if (data[i] == '(') {
		currentToken->type = LPAREN;
		currentToken->lexeme = "(";
		currentToken->length = 1;
	}
	else if (data[i] == ')') {
		currentToken->type = RPAREN;
		currentToken->lexeme = ")";
		currentToken->length = 1;
	}
	// brackets
	else if (data[i] == '{') {
		currentToken->type = LBRACK;
		currentToken->lexeme = "{";
		currentToken->length = 1;
	}
	else if (data[i] == '}') {
		currentToken->type = RBRACK;
		currentToken->lexeme = "}";
		currentToken->length = 1;
	} else if (data[i] == ',') {
		currentToken->type = COMMA;
		currentToken->lexeme = ",";
		currentToken->length = 1;
	}
	// compare
	else if (data[i] == '<') {
		if (data[i+1] == '=') {
			currentToken->type = COMPARE;
			currentToken->lexeme = "<=";
			currentToken->length = 2;
		} else {
			currentToken->type = COMPARE;
			currentToken->lexeme = "<";
			currentToken->length = 1;
		}
	} else if (data[i] == '>') {
		if (data[i+1] == '=') {
			currentToken->type = COMPARE;
			currentToken->lexeme = ">=";
			currentToken->length = 2;
		} else {
			currentToken->type = COMPARE;
			currentToken->lexeme = ">";
			currentToken->length = 1;
		}
	} else if (data[i] == '!' && data[i+1] == '=') {
		currentToken->type = COMPARE;
		currentToken->lexeme = "!=";
		currentToken->length = 2;
	} else {
		error("Unexpected token");
	}
	if (debug) {
		printf("Symbol %s at spot #%d/%ld, LEN=%d\n", currentToken->lexeme, i+1, strlen(data), currentToken->length);
	}
	i += currentToken->length;
}

void eat(int type) {
	if (currentToken->type != type) {
		printf("Expected type %d, got type %d\n", type, currentToken->type);
		exit(1);
	}
	
	nextToken();
}

int old_i = 0;
int old_length;

void lookAhead() {
	old_i = i;
	old_length = currentToken->length;
	nextToken();
}

void goBack() {
	i = old_i;
	i -= old_length;
	nextToken();
}

char * num() {
	if (currentToken->type == INTEGER) {
		char * value = currentToken->lexeme;
		char * reg = nextRegister();
		fprintf(output, "\tmovq $%s, %s\n", value, reg);
		eat(INTEGER);
		return reg;
	} else if (currentToken->type == ID) {
		char * varName = currentToken->lexeme;
		int offset = getIntOffset(varName);
		printf("\nID Offset: %d\n", offset);
		char * reg = nextRegister();
		fprintf(output, "\tmovq -%d(%%rbp), %s\n", offset, reg);
		eat(ID);
		return reg;
	} else {
		error("Can only multiply integers");
		return NULL;
	}
}

char * factor() {
	lookAhead();
	if (currentToken->lexeme[0] == '*' || currentToken->lexeme[0] == '/') {
		char * reg = NULL;
		while (currentToken->lexeme[0] == '*' || currentToken->lexeme[0] == '/') {
			char * num1;
			if (reg == NULL) {
				goBack();
				num1 = num();
				printf("NUM1");
			} else {
				num1 = reg;
			}
			
			char op = currentToken->lexeme[0];
			eat(OP);
			
			char * num2 = num();
			
			if (op == '*') {
				printf("reg1\n");
				printf(num1);
				printf(num2);
				fprintf(output, "\timul %s, %s\n", num2, num1);
				printf("reg2\n");
				freeRegister(num2);
				printf("reg3\n");
				reg = num1;
				printf("reg4\n");
			} else if (op == '/') {
				fprintf(output, "\tpushq %%rdx\n"); // save edx
				fprintf(output, "\tpushq %%rax\n"); // save eax
				fprintf(output, "\tmovq %s, %%rax\n", num1); // eax = dividend
				fprintf(output, "\tidiv %s\n", num2); // divide by divisor
				fprintf(output, "\tmovq %%rax, %s\n", num1); // put result in num1
				if (strcmp(num1, "%rax") != 0 && strcmp(num2, "%rax") != 0) {
					fprintf(output, "\tpopq %%rax\n"); // restore eax
				}
				fprintf(output, "\tpopq %%rdx\n"); // save edx
				freeRegister(num2);
				reg = num1;
			}
		}
		return reg;
	} else {
		goBack();
		char * reg = num();
		return reg;
	}
}

int lastExprType = 0;

int getType(char * varName) {
	int offset = getIntOffset(varName);
	if (offset >= 0) {
		return INTEGER;
	} else {
		offset = getStrVar(varName);
		if (offset >= 0) { return STRING; }
		else if (isFunc(varName)) {
			return FUNC;
		} else {
			return -1;
		}
	}
}

char * expr() {
	lookAhead();
	if (currentToken->type == OP) {
		char * reg = NULL;
		if (currentToken->lexeme[0] == '*' || currentToken->lexeme[0] == '/') {
			goBack();
			reg = factor();
		}
		while (currentToken->lexeme[0] == '+' || currentToken->lexeme[0] == '-') {
			char * num1;
			if (reg == NULL) {
				goBack();
				num1 = num();
			} else {
				num1 = reg;
			}
			
			char op = currentToken->lexeme[0];
			eat(OP);
			
			char * num2 = factor();
			if (op == '+') {
				fprintf(output, "\taddq %s, %s\n", num2, num1);
				freeRegister(num2);
				reg = num1;
			} else if (op == '-') {
				fprintf(output, "\tsubq %s, %s\n", num2, num1);
				freeRegister(num2);
				reg = num1;
			}
		}
		return reg;
	} else {
		goBack();
		if (currentToken->type == INTEGER) {
			char * reg = num();
			lastExprType = INTEGER;
			return reg;
		} else if (currentToken->type == ID) {
			printf("%s\n", currentToken->lexeme);
			int type = getType(currentToken->lexeme);
			char * reg;
			if (type == INTEGER) { // int
				reg = nextRegister();
				int offset = getIntOffset(currentToken->lexeme);
				fprintf(output, "\tmovq -%d(%%rbp), %s\n", offset, reg);
				lastExprType = INTEGER;
			} else if (type == STRING) {
				int loc = getStrVar(currentToken->lexeme);
				reg = nextRegister();
				fprintf(output, "\tleaq .L%d(%%rip), %s\n", loc, reg);
				lastExprType = STRING;
			} else if (type == FUNC) {
				callFunc();
				printf("called func\n");
				return "%rax";
			} else {
				printf("TYPE->%d, LEXEME: %s\n", type, currentToken->lexeme);
				error("Unexpected expression type");
			}
			
			eat(ID);
			return reg;
		} else if (currentToken->type == STRING) {
			int l = getStringLoc(currentToken->lexeme);
			char * reg = nextRegister();
			fprintf(output, "\tleaq .L%d(%%rip), %s\n", l, reg);
			eat(STRING);
			lastExprType = STRING;
			return reg;
		} else {
			error("Unexpected token in expression");
			return NULL;
		}
	}
}

void error(char * msg) {
	printf("Error: %s\n", msg);
	exit(1);
}

void print() {
	eat(PRINT);
	printf("printing");
	char * reg = expr();
	save("%rcx");
	save("%rdx");
	if (lastExprType == INTEGER) {
		fprintf(output, "\tmovq %s, %%rdx\n", reg);
		fprintf(output, "\tleaq .format(%%rip), %%rcx\n");
	} else if (lastExprType == STRING) {
		fprintf(output, "\tmovq %s, %%rcx\n", reg);
	}
	fprintf(output, "\tcall printf\n");
	restore("%rdx");
	restore("%rcx");
	eat(SEMI);
	freeRegister(reg);
}

void varLine() {
	eat(VAR);
		
	char name[50];
	memcpy(name, currentToken->lexeme, strlen(currentToken->lexeme)+1);
	
	eat(ID);
	
	eat(EQUALS);
	printf("1\n");
	int type = currentToken->type;
	printf("2\n");
	char * lexeme = currentToken->lexeme;
	printf("3\n");
	char * reg = expr();
	printf("4\n");

	if (type == ID) {
		// figure out what type it really is
		printf("LEXEME: %s\n", lexeme);
		printf(intNames[0]);
		printf("\n%d\n", getIntOffset(lexeme));
		type = getType(lexeme);
	}
	
	if (type == INTEGER) {
		addInt(name);
		int offset = getIntOffset(name);
		fprintf(output, "\tmovq %s, -%d(%%rbp)\n", reg, offset);
		printf("NEW VAR: %s (OFFSET %d)\n", name, offset);
	} else if (type == STRING) {
		addStrVar(name, getStringLoc(lexeme));
		printf("%d\n", getStringLoc("hello"));
		printf("NEW STRING VAR: %s(%d)\n", name, getStrVar(name));
	} if (type == FUNC) {
		addInt(name);
		int offset = getIntOffset(name);
		fprintf(output, "\tmovq %s, -%d(%%rbp)\n", reg, offset);
		printf("NEW VAR: %s (OFFSET %d)\n", name, offset);
	}
	
	eat(SEMI);
	freeRegister(reg);
}

void reassignVar() {
	char * name = currentToken->lexeme;
	int type = getType(name);
	
	eat(ID);
	if (type == INTEGER) {
		
		eat(EQUALS);
		
		char * newReg = expr();
		int varOffset = getIntOffset(name);
		
		fprintf(output, "\tmovq %s, -%d(%%rbp)\n", newReg, varOffset);
		
		eat(SEMI);
		freeRegister(newReg);
		
	} else if (type == STRING) {
		error("Reassigning strings is unsupported...\n");
	} else {
		error("That variable doesn't exist\n");
	}
}

int compareAssembly(char * prefix, char * reg1, char * reg2, char * operator) {
	fprintf(output, "\tcmp %s, %s\n", reg2, reg1);
	int loc = newJumpLoc();
	if (strcmp(operator, "==") == 0) {
		fprintf(output, "\tjne %s%d\n", prefix, loc);
	} else if (strcmp(operator, "!=") == 0) {
		fprintf(output, "\tje %s%d\n", prefix, loc);
	} else if (strcmp(operator, "<") == 0) {
		fprintf(output, "\tjge %s%d\n", prefix, loc);
	} else if (strcmp(operator, ">") == 0) {
		fprintf(output, "\tjle %s%d\n", prefix, loc);
	} else if (strcmp(operator, "<=") == 0) {
		fprintf(output, "\tjg %s%d\n", prefix, loc);
	} else if (strcmp(operator, ">=") == 0) {
		fprintf(output, "\tjl %s%d\n", prefix, loc);
	} else {
		error("Invalid Condition Operator");
	}
	return loc;
}

void ifStatement() {
	eat(IF);
	eat(LPAREN);
	char * reg1 = expr();
	if (lastExprType != INTEGER) {
		printf("Last expr type: %d\n", lastExprType);
		error("Can only compare integers");
	}
	
	char * operator = currentToken->lexeme;
	lookAhead();
	char * reg2 = expr();
	
	int loc = compareAssembly("jl", reg1, reg2, operator);
	int endLoc = newJumpLoc();
	
	eat(RPAREN);
	eat(LBRACK);
	lines();
	eat(RBRACK);
	
	fprintf(output, "\tjmp jl%d\n", endLoc); // if it got here, the whole if statement was executed
	fprintf(output, "\tjl%d:\n", loc);
	
	freeRegister(reg2);
	freeRegister(reg1);
	
	while (currentToken->type == ELIF) {
		eat(ELIF);
		eat(LPAREN);
		
		reg1 = expr();
		if (lastExprType != INTEGER) {
			error("Can only compare integers");
		}
		
		operator = currentToken->lexeme;
		lookAhead();
		reg2 = expr();
		
		int elifLoc = compareAssembly("jl", reg1, reg2, operator);
		eat(RPAREN);
		eat(LBRACK);
		lines();
		eat(RBRACK);
		fprintf(output, "\tjmp jl%d\n", endLoc); // elseif was true
		fprintf(output, "\tjl%d:\n", elifLoc);
		freeRegister(reg1);
		freeRegister(reg2);
		
	}
	
	if (currentToken->type == ELSE) {
		eat(ELSE);
		eat(LBRACK);
		lines();
		eat(RBRACK);
	}
	
	fprintf(output, "\tjl%d:\n", endLoc);
}

void forLoop() {
	eat(FOR);
	
	eat(LPAREN);
	if (currentToken->type != ID) { error("Unexpected token in for definition"); }
	char * counter = currentToken->lexeme;
	addInt(counter);
	eat(ID);
	
	printf(currentToken->lexeme);
	
	eat(INRANGE);
	char * start = expr();
	eat(COMMA);
	char * end = expr();
	
	eat(RPAREN);
	
	fprintf(output, "\tmovq %s, -%d(%%rbp)\n", start, getIntOffset(counter));
	freeRegister(start);
	int loc = newJumpLoc();
	int endLoc = newJumpLoc();
	fprintf(output, "\tcmp %s, -%d(%%rbp)\n", end, getIntOffset(counter));
	fprintf(output, "\tjge f%d\n", endLoc);
	fprintf(output, "\tf%d:\n", loc);
	
	eat(LBRACK);
	lines();
	eat(RBRACK);
	
	fprintf(output, "\taddq $1, -%d(%%rbp)\n", getIntOffset(counter));
	fprintf(output, "\tcmp %s, -%d(%%rbp)\n", end, getIntOffset(counter));
	fprintf(output, "\tjl f%d\n", loc);
	fprintf(output, "\tf%d:\n", endLoc);
	
	freeRegister(end);
	
}

void whileLoop() {
	eat(WHILE);
	eat(LPAREN);
	
	int startLoc = newJumpLoc();
	fprintf(output, "\twls%d:\n", startLoc);
	
	char * reg1 = expr();
	if (lastExprType != INTEGER) {
		error("Can only compare integers");
	}
	
	char * operator = currentToken->lexeme;
	lookAhead();
	char * reg2 = expr();
	
	
	int loc = compareAssembly("wl", reg1, reg2, operator);
	
	eat(RPAREN);
	eat(LBRACK);
	lines();
	eat(RBRACK);
	
	fprintf(output, "\tjmp wls%d\n", startLoc);
	fprintf(output, "\twl%d:\n", loc);
	
}

void function() {
	eat(FUNC);
	char * name = currentToken->lexeme;
	eat(ID);
	
	fprintf(output, "%s:\n", name);
	fprintf(output, "\tpushq %%rbp\n\tmovq %%rsp, %%rbp\n\tsubq $128, %%rsp\n");
	if (strcmp(name, "main") == 0) {
		fprintf(output, "\tcall __main\n");
	}
	
	addFuncName(name);
	
	eat(LPAREN);
	
	char * argRegs[] = {"%rcx", "%rdx", "%r8d", "%r9d"};
	int argNum = 0;
	while (currentToken->type == ID) {
		char * argName = currentToken->lexeme;
		
		// add argument as variable
		addInt(argName);
		int offset = getIntOffset(argName);
		fprintf(output, "\tmovq %s, -%d(%%rbp)\n", argRegs[argNum], offset);
		printf("NEW ARG VAR: %s (OFFSET %d)\n", argName, offset);
		
		eat(ID);
		if (currentToken->type == RPAREN) {
			break;
		}
		eat(COMMA);
		argNum++;
	}
	
	eat(RPAREN);
	eat(LBRACK);
	lines();
	eat(RBRACK);
	
	fprintf(output, "\tmovq $0, %%rax\n\taddq $128, %%rsp\n\tpopq %%rbp\n\tret\n\n");
}

void callFunc() {
	char * name = currentToken->lexeme;
	eat(ID);
	eat(LPAREN);
	
	char * argRegs[] = {"%rcx", "%rdx", "%r8d", "%r9d"};
	int argNum = 0;
	while (currentToken->type == INTEGER || currentToken->type == ID) {
		printf("first arg");
		char * argReg = expr();
		printf("first arg done");
		fprintf(output, "\tmovq %s, %s\n", argReg, argRegs[argNum]);
		if (currentToken->type == RPAREN) {
			break;
		}
		eat(COMMA);
		argNum++;
	}
	
	eat(RPAREN);
	
	fprintf(output, "\tcall %s\n", name);
}

void returnLine() {
	eat(RETURN);
	
	char * reg = expr();
	fprintf(output, "\tmovq %s, %%rax\n", reg);
	fprintf(output, "\taddq $128, %%rsp\n");
	fprintf(output, "\tpopq %%rbp\n");
	fprintf(output, "\tret\n");
	eat(SEMI);
	
}

void line() {
	if (currentToken->type == VAR) {
		printf("VARLINE\n");
		varLine();
	} else if (currentToken->type == PRINT) {
		print();
	} else if (currentToken->type == IF) {
		ifStatement();
	} else if (currentToken->type == ID) {
		lookAhead();
		if (currentToken->type == EQUALS) {
			goBack();
			reassignVar();
		} else if (currentToken->type == LPAREN) {
			goBack();
			callFunc();
			eat(SEMI);
		} else {
			printf("\nan: %d\n", currentToken->type);
			error("Unexpected alphanumeric");
		}
	} else if (currentToken->type == FOR) {
		forLoop();
	} else if (currentToken->type == WHILE) {
		whileLoop();
	} else if (currentToken->type == FUNC) {
		function();
	} else if (currentToken->type == RETURN) {
		returnLine();
	} else {
		printf("not working i guess\n");
		printf("Unexpected token %s(TYPE %d)\n", currentToken->lexeme, currentToken->type);
		exit(1);
	}
}

void lines() {
	while (currentToken->type != EOF && currentToken->type != RBRACK) {line();}
}

void preprocess(char * data) {
	printf("Preprocessing\n");
	int isString = 0;
	char * currentString = malloc(sizeof(currentString));
	int index = 0;
	for (int i = 0; i < strlen(data); i++) {
		
		if (data[i] == '\"') {
			if (isString == 1) {
				isString = 0;
				currentString[index] = '\0';
				printf("\n%s\n", currentString);
				int loc = addString(currentString);
				printf("\n%s\n", currentString);
				fprintf(output, ".L%d:\n", loc);
				fprintf(output, "\t.ascii \"%s\\12\\0\"\n", currentString);
				
				printf("New String: %s(%d)\n", currentString, getStringLoc(currentString));
				free(currentString);
				currentString = malloc(sizeof(currentString));
				index = 0;
				
			}
			else { isString = 1; }
		} else if (isString) {
			currentString[index] = data[i];
			currentString[index+1] = '\0';
			index++;
			printf(currentString);
		}
		
	}
}

int main(int argc, char **argv) {
	
	if (argc < 2) {
		printf("Please specify an input file.\n");
		return 1;
	} else {
		if (access(argv[1], F_OK) < 0) {
			printf("File not found.\n");
			return 2;
		}
	}
	
	if (argc == 3 && strcmp(argv[2], "-s") == 0) {
		debug = 1;
	}
	
	data = malloc(getFileLength(argv[1]));
	strcpy(data, readlines(argv[1]));
	
	char* base = strtok(argv[1], ".");
	
	char outputFileName[50];
	sprintf(outputFileName, "%s.s", base);
	
	output = fopen(outputFileName, "wb");
	fprintf(output, "\t.globl main\n");
	fprintf(output, ".format:\n\t.ascii \"%%d\\12\\0\"\n");
	preprocess(data);
	
	currentToken = malloc(sizeof(currentToken));
	nextToken();
	
	lines();
	
	fclose(output);
	
	char cmd[100];
	sprintf(cmd, "gcc %s.s -o %s", base, base);
	system(cmd);
	
	if (debug == 0) {
		char delCMD[100];
		sprintf(delCMD, "rm %s.s", base);
		printf(delCMD);
		system(delCMD);
	}
	
	return 0;
}