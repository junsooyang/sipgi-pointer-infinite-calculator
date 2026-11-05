#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>

/* 문자 하나를 담는 연결리스트 노드 */
typedef struct _node {
	char data;
	struct _node *next;
}Node, *pNode;

/* 노드들을 묶은 연결리스트 (입력식, 후위식, 결과를 모두 이걸로 저장) */
typedef struct _list {
	int count;
	pNode head;
	pNode tail;
}List, *pList;

// linklist.c : 연결리스트 조작
pList makeList(void);
pNode makeNode(char);
void insertFront(pList, pNode);
void insertBack(pList, pNode);
void insertMid(pList, int, pNode);
void deleteHead(pList);
void deleteTail(pList);
void removeSpace(pList);
void removeAllSpace(pList);
int  searchNode(pList, char);
void printList(pList);
void freeList(pList);

// stack.c : 연산자 스택 (후위표기 변환용)
struct node;
struct node* new_node(int data);
int  empty(struct node* head);
void push(struct node** head, int data);
int  pop(struct node** head);
int  top(struct node* head);

// 처리 단계
int   check(pList);   // check.c    입력이 올바른 식인지 검사 (1: 정상, 0: 오류)
void  change(pList);  // change.c   공백 제거, 부호 처리(-5 → 0-5), 생략 곱셈(3(4) → 3*(4))
pList postfix(pList); // postfix.c  중위식 → 후위식 변환
pList seccalc(pList); // rational.c 후위식을 분수로 계산해 결과 문자열 리스트 반환

extern int calcError; // rational.c  0: 정상, 1: 0으로 나눔, 2: 잘못된 식
