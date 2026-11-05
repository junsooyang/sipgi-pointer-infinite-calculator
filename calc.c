#include "calc.h"

int main(int argc, char* argv[]){
	//1.    파일 입력받아서 연결리스트 저장받기
	if (argc < 2) {
		fprintf(stderr, "usage: %s <input file>\n", argv[0]);
		return 1;
	}
	FILE *pFile = fopen(argv[1], "r");
	if (pFile == NULL) {
		fprintf(stderr, "cannot open file: %s\n", argv[1]);
		return 1;
	}

	pList expr = makeList();
	int ch; // EOF(-1) 와 문자 0xFF 를 구분하려고 int 로 받는다
	int first = 1;
	// 파일 끝까지 읽는다. 여러 줄이면 줄바꿈을 공백으로 바꿔 한 식으로 이어 붙인다.
	while ((ch = fgetc(pFile)) != EOF) {
		/* 맨 앞 UTF-8 BOM(EF BB BF) 은 건너뛴다 (메모장 저장 파일 대비) */
		if (first && ch == 0xEF) {
			int c2 = fgetc(pFile), c3 = fgetc(pFile);
			if (c2 == 0xBB && c3 == 0xBF) { first = 0; continue; }
			insertBack(expr, makeNode((char)ch));
			if (c2 != EOF) insertBack(expr, makeNode((char)c2));
			if (c3 != EOF) insertBack(expr, makeNode((char)c3));
			first = 0;
			continue;
		}
		first = 0;
		// 줄바꿈/탭/캐리지리턴/수직탭/폼피드 등 공백류 제어 문자는 공백으로 통일.
		// (이후 removeAllSpace 가 제거하므로 "1+2\n+3" 은 "1+2 +3" -> "1+2+3" 으로 이어진다)
		if (ch == '\n' || ch == '\t' || ch == '\r' || ch == '\v' || ch == '\f') ch = ' ';
		// 그 외 제어 문자(0x00~0x1F)는 무시 (공백류는 위에서 이미 ' ' 로 바뀜)
		if (ch >= 0 && ch < ' ') continue;
		insertBack(expr, makeNode((char)ch));
	}
	fclose(pFile);

	//       2.메시지 출력하기
	printf("====== Welcome to infinite calculator! ======\n");
	printf("Enter the expression in infix notation.\n");
	printf("Input : ");
	printList(expr);
	//       3.중위표기식인지 확인하기 (빈 입력도 잘못된 식)
	int exitCode = 0; // 0: 정상, 1: 오류(식 오류/0 나누기 포함). 에러 메시지는 stderr 로 출력.
	if (expr->head != NULL && check(expr))
	{
		//       4.중위표기식 다듬기 (0 추가 등)
		change(expr);

		//       5.중위표기식을 후위표기식으로 변환하기
		pList pos = postfix(expr);

		//       6.후위표기식을 계산하기
		pList answer = seccalc(pos);

		//       7.결과 출력하기
		if (calcError == 1) { fprintf(stderr, "divide by zero\n"); exitCode = 1; }
		else if (calcError == 2) { fprintf(stderr, "wrong expression\n"); exitCode = 1; }
		else {
			printf("Result : ");
			printList(answer);
		}
		freeList(pos);
		free(pos);
		freeList(answer);
		free(answer);
	}
	else {
		fprintf(stderr, "wrong expression\n");
		exitCode = 1;
	}
	freeList(expr);
	free(expr);
	return exitCode;
}
