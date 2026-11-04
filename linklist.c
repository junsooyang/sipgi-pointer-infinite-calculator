#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _node {
	char data;
	struct _node *next;
}Node, *pNode;

typedef struct _list {
	int count;  // 노드의 갯수
	pNode head; // 첫번째 노드
	pNode tail; // 마지막 노드
}List, *pList;

//Linked List를 만들기 위한 함수. 
pList makeList() {
	pList list = (pList)malloc(sizeof(List));
	list->count = 0;
	list->head = NULL;
	list->tail = NULL;
	return list;
}

//Node를 만드는 함수. Data를 입력 받아서 저장하고, 만든 node를 반환한다.
pNode makeNode(char data) {
	pNode node = (pNode)malloc(sizeof(Node));
	node->data = data;
	node->next = NULL;
	return node;
}

//집어 넣을 노드인 N을 List L의 가장 처음, 즉 head에 집어 넣는 함수.
void insertFront(pList L, pNode N) {
	if(L->head == NULL) {
		L->head = N;
		L->tail = N;
		N->next = NULL;
	}
	else {
		N->next = L->head;
		L->head = N;
	}
	(L->count)++;
}

int isNode(pList L, char data) {
	pNode tmp = L->head;
	while(tmp != NULL) {
		if (tmp->data == data) {
			return 1;
		}
		tmp=tmp->next;
	}
	return 0;
}

int searchNode(pList L, char data) {
	pNode tmp = L->head;
	int count = 1;
	while(tmp != NULL) {
		if(tmp->data == data) {
			return count;
		}
		tmp = tmp->next;
		count++;
	}
}

void insertBack(pList L, pNode N) {
	if(L->head == NULL) {
		L->head = N;
		L->tail = N;
		N->next = NULL;
	}
	else {
		N->next = NULL;
		L->tail->next = N;
		L->tail = N;
	}
	(L->count)++;
}

void printList(pList L) {
	if (L->head == NULL) { printf("\n"); return; } // 빈 리스트
	pNode tmp = L->head;
	for (size_t i = 1; i < L->count; i++)
	{
		printf("%c", tmp->data);
		tmp = tmp->next;
	}
	printf("%c\n", tmp->data);
	//printf("Head : %d, Tail : %d", L->head->data, L->tail->data);
}

/* data 와 같은 노드를 모두 삭제 */
void deleteData(pList L, int data) {
	pNode prev = NULL;
	pNode tmp = L->head;
	while (tmp != NULL) {
		pNode next = tmp->next;
		if (tmp->data == data) {
			if (prev == NULL) L->head = next;
			else prev->next = next;
			if (tmp == L->tail) L->tail = prev;
			free(tmp);
			(L->count)--;
		}
		else {
			prev = tmp;
		}
		tmp = next;
	}
}

void deleteHead(pList L) {
	if (L->head == NULL) ;
	else if (L->head == L->tail) {
		free(L->head);
		L->head = NULL;
		L->tail = NULL;
		(L->count)--;
	}
	else {
		pNode old = L->head;
		L->head = old->next;
		free(old);
		(L->count)--;
	}
}

void deleteTail(pList L) {
	if (L->head == NULL) ;
	else if (L->head == L->tail) {
		free(L->head);
		L->head = NULL;
		L->tail = NULL;
		(L->count)--;
	}
	else {
		pNode temp = L->head;
		while ((temp->next) != (L->tail)) {
			temp = temp->next;
		}
		free(temp->next);
		temp->next = NULL;
		L->tail = temp;
		(L->count)--;
	}
}

/* 앞뒤 공백을 지우고, 연속된 공백은 하나로 줄인다 */
void removeSpace(pList L) {
	while (L->head != NULL && L->head->data == ' ') deleteHead(L);
	while (L->tail != NULL && L->tail->data == ' ') deleteTail(L);
	/* 앞쪽 노드를 지운 뒤에 순회를 시작해야 해제된 노드를 건드리지 않는다 */
	pNode temp = L->head;
	while (temp != NULL && temp->next != NULL) {
		if (temp->data == ' ' && temp->next->data == ' ') {
			pNode dup = temp->next;
			temp->next = dup->next;
			if (dup == L->tail) L->tail = temp;
			free(dup);
			(L->count)--;
		}
		else {
			temp = temp->next;
		}
	}
}

void removeZero(pList L) {
	pNode temp = L->head;
	pNode prev;
	while (L->head->data == '0') {
		deleteHead(L);
	}
	while (L->tail->data == '0') {
		deleteTail(L);
	}
}

void insertMid(pList L,int index,pNode N) {
	pNode temp = L->head;
	for (int i = 1; i<index; i++) {
		temp = temp->next;
	}
	N->next = temp->next;
	temp->next = N;
	(L->count)++;
}

int compareList(pList a,pList b) {
	pNode a_temp = a->head;
	pNode b_temp = b->head;
	while(a_temp != NULL) {
		if(a_temp->data - b_temp->data > 0) return 1;
		else if(a_temp->data - b_temp->data < 0) return 0;
		a_temp = a_temp->next;
		b_temp = b_temp->next;
	}
	return 1;
}	

void freeList(pList L) {
	pNode tmp = L->head;
	pNode prev = NULL;
	for (size_t i = 0; i < L->count; i++)
	{
		prev = tmp;			
		tmp = tmp->next;	
		free(prev);			
	}
}

/*
void main() {
	pList list = makeList();
	char expr[] = "12356";
	for (int i = 0; i<strlen(expr); i++) {
		insertBack(list, makeNode(expr[i]));
	}
	printList(list);
	printf("%d %d %d\n",isNode(list,'1'),isNode(list,'4'),isNode(list,'6'));
	printf("%d\n",searchNode(list,'3'));
	printList(list);
	freeList(list);
	free(list);
}
*/
