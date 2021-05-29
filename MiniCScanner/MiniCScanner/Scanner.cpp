/***************************************************************
*      scanner routine for Mini C language                    *
***************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "Scanner.h"

extern FILE* sourceFile;                       // miniC source program


int superLetter(char ch);
int superLetterOrDigit(char ch);
int getNumber(char firstCharacter);
int hexValue(char ch);
void lexicalError(int n);

double getFractional(double n) {
	while (n > 1)
		n /= 10.0;

	return n;
}

int col = 1, line = 1;

char* tokenName[] = {
	"!",        "!=",      "%",       "%=",     "%ident",   "%number",
	/* 0          1           2         3          4          5        */
	"&&",       "(",       ")",       "*",      "*=",       "+",
	/* 6          7           8         9         10         11        */
	"++",       "+=",      ",",       "-",      "--",	    "-=",
	/* 12         13         14        15         16         17        */
	"/",        "/=",      ";",       "<",      "<=",       "=",
	/* 18         19         20        21         22         23        */
	"==",       ">",       ">=",      "[",      "]",        "eof",
	/* 24         25         26        27         28         29        */
	//   ...........    word symbols ................................. //
	/* 30         31         32        33         34         35        */
	"const",    "else",     "if",      "int",     "return",  "void",
	/* 36         37         38        39         40         41        */
	"while",    "{",        "||",      "}",      "char",   "double",
	/* 42         43         44        45         46         47      */
	 "for",      "%do",      "goto",   "switch",    "case",     "break",
	 /* 48         49         50        51         52*/
	 "default",    ":",     "%char", "%string", "%double"

};

char* keyword[NO_KEYWORD] = {
	"const",  "else",    "if",    "int",    "return",  "void",    "while",

	//�߰� Ȯ�� �κ�
	"char",   "double",  "for",   "do",     "goto",    "switch",  "case",

	"break",  "default"
};


enum tsymbol tnum[NO_KEYWORD] = {
	tconst,    telse,     tif,     tint,     treturn,   tvoid,     twhile,
	//�߰� Ȯ�� �κ�
	tchar,     tdouble,   tfor,    tdo,      tgoto,     tswitch,   tcase,

	tbreak,    tdefault
};

struct tokenType scanner()
{
	struct tokenType token;
	int i, index;
	char ch, id[ID_LENGTH];

	token.number = tnull;

	do {
		while (isspace(ch = fgetc(sourceFile))) {	// state 1: skip blanks isspace->�����̸� 0��ȯ, fgetc �ѱ��ھ� �ε�
			col++;
			if (ch == '\n')
			{
				col = 0;
				line++;
			}
		}
		col++;

		if (superLetter(ch)) { // identifier or keyword
			i = 0;
			token.colno = col - 1;
			do {
				if (i < ID_LENGTH) id[i++] = ch;
				ch = fgetc(sourceFile);
				col++;
			} while (superLetterOrDigit(ch));
			if (i >= ID_LENGTH) lexicalError(1);
			id[i] = '\0';
			ungetc(ch, sourceFile);  //  retract
			col--;
			// find the identifier in the keyword table
			for (index = 0; index < NO_KEYWORD; index++)
				if (!strcmp(id, keyword[index])) break;
			if (index < NO_KEYWORD)    // found, keyword exit
			{
				token.number = tnum[index];
				token.colno = col - strlen(tokenName[token.number]);
				token.lineno = line;
			}
			else {                     // not found, identifier exit
				token.number = tident;
				strcpy_s(token.value.id, id);
			}
		}  // end of identifier or keyword

		else if (isdigit(ch)) {  // number
			token.colno = col - 1;
			token.value.num = getNumber(ch);
			ch = fgetc(sourceFile);
			col++;
			if (ch == '.') { //double�� ���
				double tmp;
				token.number = tdoubles; //token number ����
				ch = fgetc(sourceFile);
				if (isdigit(ch)) {

					ungetc(ch, sourceFile);
					col--;
					if (ch == '0') { //���� ���ڰ� 0�� ���, ex) 2.04 ��
						ch = fgetc(sourceFile);
						col++;
						token.value.d_num = token.value.num + 0.1 * getFractional(getNumber(ch)); //�̹� ���ڿ� 0.1�� ���Ѵ�.
						ch = fgetc(sourceFile);
						col++;
					}
					else { //���� ���ڰ� 0�� �ƴ� ���,
						ch = fgetc(sourceFile);
						col++;
						token.value.d_num = token.value.num + getFractional(getNumber(ch));
						ch = fgetc(sourceFile);
						col++;
					}

					if (ch == 'e') { //�ε��Ҽ���
						ch = fgetc(sourceFile);
						col++;
						if (isdigit(ch)) {
							index = getNumber(ch);
							for (i = 0; i < index; i++)
								token.value.d_num *= 10;
						}
						else if (ch == '+') {
							ch = fgetc(sourceFile);
							col++;
							index = getNumber(ch);
							for (i = 0; i < index; i++)
								token.value.d_num *= 10; //+���� ����ŭ 10�� ������
						}
						else if (ch == '-') {
							ch = fgetc(sourceFile);
							col++;
							index = getNumber(ch);
							for (i = 0; i < index; i++)
								token.value.d_num /= 10; //-���� ����ŭ 10���� ������.
						}
						else
						{
							ungetc(ch, sourceFile);
							col--;
						}

					}
					else //�ε��Ҽ����� �ƴ� ���,
					{
						ungetc(ch, sourceFile);
						col--;
					}
				}
				else if (ch == ';') { //X. ���
					token.value.d_num = token.value.num;
					ungetc(ch, sourceFile);
					col--;
				}
			}
			else // Integer��
			{
				token.number = tnumber;
				ungetc(ch, sourceFile);
				col--;
			}
		}
		else if (ch == '.') // .(dot)�� �Ǿ��� ��� ex) .24
		{
			token.colno = col - 1;
			ch = fgetc(sourceFile);
			col++;
			if (isdigit(ch)) {
				token.number = tdoubles;
				token.value.num = 0;
				token.value.d_num = getFractional(getNumber(ch));
			}
			else { //���� ���ڰ� ���ڰ� �ƴ϶�� ���� ���
				lexicalError(7);
			}
		}
		else
		{
			token.colno = col - 1;
			switch (ch) {  // special character

			case '/': //Comments
				ch = fgetc(sourceFile);
				col++;
				if (ch == '*') //text or Documented Comments
				{
					ch = fgetc(sourceFile);
					col++;
					if (ch == '\n') {
						col = 0;
						line++;
					}
					if (ch == '*') //Documented Comment �� ��,
					{
						printf("Documented Comments ------> ");
						do {
							while (ch != '*') //Comment ���� ���
							{

								printf("%c", ch);
								ch = fgetc(sourceFile);
								if (ch == '\n') {
									col = 0;
									line++;
								}
								else
									col++;

							}
							ch = fgetc(sourceFile);
							if (ch == '\n') {
								col = 0;
								line++;
							}
							else
								col++;
						} while (ch != '/');
						printf("\n");

					}
					else // �Ϲ� text Comment �� ��,
					{
						ungetc(ch, sourceFile);
						col--;
						do {
							while (ch != '*')
							{
								ch = fgetc(sourceFile);
								if (ch == '\n') {
									col = 0;
									line++;
								}
								else
									col++;
							}
							ch = fgetc(sourceFile);
							if (ch == '\n') {
								col = 0;
								line++;
							}
							else
								col++;
						} while (ch != '/');
					}

				}

				else if (ch == '/') //line or Single Documented Commenets
				{
					ch = fgetc(sourceFile);
					col++;
					if (ch == '/') //Single Documented Comments �� ��,
					{
						printf("Single Documented Comments ------> ");

						while (ch != '\n') //Comment ���� ���
						{
							ch = fgetc(sourceFile);
							if (ch == '\n') {
								col = 0;
								line++;
							}
							else
								col++;
							printf("%c", ch);

						}

						printf("\n");
					}
					else // line comment
					{
						ungetc(ch, sourceFile);
						while (fgetc(sourceFile) != '\n')
						{
							col++;
						}
						line++;
						col = 0;
					}
				}
				else if (ch == '=')  token.number = tdivAssign;
				else {
					token.number = tdiv;
					ungetc(ch, sourceFile); // retract
					col--;
				}
				break;
			case '!':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '=')  token.number = tnotequ;
				else {
					token.number = tnot;
					ungetc(ch, sourceFile); // retract
					col--;
				}
				break;
			case '%':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '=') {
					token.number = tremAssign;
				}
				else {
					token.number = tremainder;
					ungetc(ch, sourceFile);
					col--;
				}
				break;
			case '&':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '&')  token.number = tand;
				else {
					lexicalError(2);
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;
			case '*':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '=')  token.number = tmulAssign;
				else {
					token.number = tmul;
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;
			case '+':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '+')  token.number = tinc;
				else if (ch == '=') token.number = taddAssign;
				else {
					token.number = tplus;
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;
			case '-':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '-')  token.number = tdec;
				else if (ch == '=') token.number = tsubAssign;
				else {
					token.number = tminus;
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;
			case '<':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '=') token.number = tlesse;
				else {
					token.number = tless;
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;
			case '=':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '=')  token.number = tequal;
				else {
					token.number = tassign;
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;
			case '>':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '=') token.number = tgreate;
				else {
					token.number = tgreat;
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;
			case '|':
				ch = fgetc(sourceFile);
				col++;
				if (ch == '|')  token.number = tor;
				else {
					lexicalError(3);
					ungetc(ch, sourceFile);  // retract
					col--;
				}
				break;

			case '\'': //Character
				ch = fgetc(sourceFile);
				col++;
				if (ch == '\\') //Escape ���ڰ� ���� Case,
				{
					token.value.id[0] = ch; //id�� Escape �� ����
					token.value.id[1] = fgetc(sourceFile);
					token.value.id[2] = '\0'; //char �迭�̱� ������, NULL�� ����				
				}
				else { //Escape ���ڰ� �ƴ� ��,
					token.value.id[0] = ch; //id�� �� ����
					token.value.id[1] = '\0';
				}
				ch = fgetc(sourceFile);
				col++;
				if (ch == '\'') //���� ' �� �� ����.
					token.number = tchars;
				else            // '�� �ƴϸ� ���� ���.
					lexicalError(5);
				break;

			case '\"': //String
				i = 0;
				while ((ch = fgetc(sourceFile)) != '\"' || //��(")�� ���� �� ����,
					id[i - 1] == '\\')                  //�Ǵ� Escape ������ \" �� ������ ��,
				{
					col++;
					if (i < ID_LENGTH)  id[i++] = ch;

					else {             //ID_LENGTH ���� ���ų� Ŀ�� ��,
						lexicalError(1);//ID_LENGTH �ʰ� �� ����(1)
						break;
					}
					if (ch == ';') { // ��(")���� ;�� ���� ���

						ungetc(ch, sourceFile); //�� ���� �������� �̵�
						col--;
						lexicalError(6); // string error ���
						break;
					}
				}
				if (i < ID_LENGTH) { //���̰� ����
					id[i] = '\0'; //NULL ����

					if (ch == '\"') { //���� ������ ���			
						strcpy_s(token.value.id, id); //value�� id�� ����
						token.number = tstring; //token�� number ����
					}
				}
				else { //���̰� ������			
					   // �� ���� line ���� �о���̱� ���ؼ�,
					while ((ch = fgetc(sourceFile)) != '\n')
					{
						col++;
					}
				}
				break;


			case '(': token.number = tlparen;         break;
			case ')': token.number = trparen;         break;
			case ',': token.number = tcomma;          break;
			case ';': token.number = tsemicolon;      break;
			case '[': token.number = tlbracket;       break;
			case ']': token.number = trbracket;       break;
			case '{': token.number = tlbrace;         break;
			case '}': token.number = trbrace;         break;
			case EOF: token.number = teof;            break;
				//�߰� Ȯ�� colon ������
			case ':': token.number = tcolon;          break;

			default: {
				printf("Current character : %c", ch);
				lexicalError(4);
				break;
			}

			} // switch end
		}
		token.lineno = line;
	} while (token.number == tnull);
	return token;
} // end of scanner

void lexicalError(int n)
{
	printf(" *** Lexical Error : ");
	switch (n) {
	case 1: printf("an identifier length must be less than 12.\n");
		break;
	case 2: printf("next character must be &\n");
		break;
	case 3: printf("next character must be |\n");
		break;
	case 4: printf("invalid character\n");
		break;
	case 5: printf("next character must be \'\n"); //character ó�� ��, �� ���� Error
		break;
	case 6: printf("next character must be \"\n"); //string ó�� ��, �� ���� Error
		break;
	case 7: printf("next character must be Number\n"); //double ó�� ��, �� ���� Error
	}
}

int superLetter(char ch)
{
	if (isalpha(ch) || ch == '_') return 1;
	else return 0;
}

int superLetterOrDigit(char ch)
{
	if (isalnum(ch) || ch == '_') return 1;
	else return 0;
}

int getNumber(char firstCharacter)
{
	int num = 0;
	int value;
	char ch;

	if (firstCharacter == '0') {
		ch = fgetc(sourceFile);
		if ((ch == 'X') || (ch == 'x')) {		// hexa decimal
			while ((value = hexValue(ch = fgetc(sourceFile))) != -1)
				num = 16 * num + value;
		}
		else if ((ch >= '0') && (ch <= '7'))	// octal
			do {
				num = 8 * num + (int)(ch - '0');
				ch = fgetc(sourceFile);
			} while ((ch >= '0') && (ch <= '7'));
		else num = 0;						// zero
	}
	else {									// decimal
		ch = firstCharacter;
		do {
			num = 10 * num + (int)(ch - '0');
			ch = fgetc(sourceFile);
		} while (isdigit(ch));
	}
	ungetc(ch, sourceFile);  /*  retract  */
	return num;
}



int hexValue(char ch)
{
	switch (ch) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return (ch - '0');
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		return (ch - 'A' + 10);
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return (ch - 'a' + 10);
	default: return -1;
	}
}

void printToken(struct tokenType token)
{
	if (token.number == tident)                                                                          //�����̸��� line ��, column ��ġ�� ��� �Ѵ�.
		printf("number: %d, value: %s, filename: %s, line: %d, col: %d\n", token.number, token.value.id, token.filename, token.lineno, token.colno);
	else if (token.number == tnumber)
		printf("number: %d, value: %d, filename: %s, line: %d, col: %d\n", token.number, token.value.num, token.filename, token.lineno, token.colno);
	else if (token.number == tchars || token.number == tstring) // character, string ���
		printf("number: %d, value: %s, filename: %s, line: %d, col: %d\n", token.number, token.value.id, token.filename, token.lineno, token.colno);
	else if (token.number == tdoubles)                          // double ���
		printf("number: %d, value: %.3lf, filename: %s, line: %d, col: %d\n", token.number, token.value.d_num, token.filename, token.lineno, token.colno);
	else
		printf("number: %d(%s), filename: %s, line: %d, col: %d\n", token.number, tokenName[token.number], token.filename, token.lineno, token.colno);

}