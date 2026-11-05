#include <stdio.h> // printf, fopen, fgetc  → 화면 출력, 파일 읽기
#include <string.h> // strlen, memcpy        → 문자열 길이, 메모리 복사
#include <stdlib.h> // malloc, free          → 동적 메모리 할당/해제
#include <limits.h> // INT_MIN               → 스택이 비었을 때 돌려주는 값
#include <ctype.h> // isdigit               → 글자가 숫자인지 검사

typedef struct _node {
	char data;
	struct _node *next;
}Node, *pNode;

typedef struct _list {
	int count;
	pNode head;
	pNode tail;
}List, *pList;

// linklist.c
pList makeList(void);
pNode makeNode(char);
void insertFront(pList, pNode);
void insertBack(pList, pNode);
void printList(pList);
void deleteData(pList,char);
void freeList(pList);
void deleteHead(pList);
void deleteTail(pList);
void removeSpace(pList);
void removeAllSpace(pList);
void removeZero(pList);
void insertMid(pList,int,pNode);
int isNode(pList,char);
int searchNode(pList,char);
int compareList(pList,pList);
pList postfix(pList expr); // postfix.c  중위식 → 후위식으로 변환한 새 리스트를 돌려줌

// stack.c
struct node;
struct node* new_node(int data);
int empty(struct node* head);
void push(struct node** head, int data);
int pop(struct node** head);
int top(struct node* head);
void display(struct node* head);

// main
int check(pList); // check.c    입력이 올바른 식인지 검사 → 맞으면 1, 틀리면 0
pList seccalc(pList); // rational.c 후위식을 계산해서 결과 글자 리스트를 돌려줌
void change(pList); // change.c   부호 처리(-5 → 0-5), 곱셈 생략 보완(3(4) → 3*(4))

struct nodeL {
	pList data;
	struct nodeL* next;
};
struct nodeL* new_nodeL(pList);
int emptyL(struct nodeL*);
void pushL(struct nodeL**,pList);
pList popL(struct nodeL**);
pList topL(struct nodeL*);
void displayL(struct nodeL*);

pList add(pList,pList);
pList minus(pList,pList);
pList multiply(pList,pList);

/* rational.c : 분수(유리수) 기반 정확 계산 */
extern int calcError;      // 0: 정상, 1: 0으로 나눔, 2: 잘못된 식
