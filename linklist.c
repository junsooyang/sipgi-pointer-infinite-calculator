#include "calc.h"

// 빈 연결리스트를 만든다.
pList makeList() {
	pList list = (pList)malloc(sizeof(List));
	if (list == NULL) { fprintf(stderr, "out of memory\n"); exit(1); }
	list->count = 0;
	list->head = NULL;
	list->tail = NULL;
	return list;
}

// 글자 하나를 담은 노드를 만든다.
pNode makeNode(char data) {
	pNode node = (pNode)malloc(sizeof(Node));
	if (node == NULL) { fprintf(stderr, "out of memory\n"); exit(1); }
	node->data = data;
	node->next = NULL;
	return node;
}

// 노드 N 을 리스트 맨 앞에 넣는다.
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

// 노드 N 을 리스트 맨 뒤에 넣는다.
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

// index 번째 노드 뒤에 N 을 끼워 넣는다 (1부터 셈).
void insertMid(pList L, int index, pNode N) {
	pNode temp = L->head;
	for (int i = 1; i < index && temp != NULL; i++) {
		temp = temp->next;
	}
	if (temp == NULL) { insertBack(L, N); return; } // 범위를 벗어나면 맨 뒤에 추가
	N->next = temp->next;
	temp->next = N;
	if (temp == L->tail) L->tail = N; // 마지막 노드 뒤에 넣었으면 tail 갱신
	(L->count)++;
}

// data 와 같은 문자를 가진 첫 노드의 위치(1부터)를 반환. 없으면 0.
int searchNode(pList L, char data) {
	pNode tmp = L->head;
	int count = 1;
	while(tmp != NULL) {
		if(tmp->data == data) return count;
		tmp = tmp->next;
		count++;
	}
	return 0;
}

// 맨 앞 노드를 삭제한다.
void deleteHead(pList L) {
	if (L->head == NULL) return;
	if (L->head == L->tail) {
		free(L->head);
		L->head = NULL;
		L->tail = NULL;
	}
	else {
		pNode old = L->head;
		L->head = old->next;
		free(old);
	}
	(L->count)--;
}

// 맨 뒤 노드를 삭제한다.
void deleteTail(pList L) {
	if (L->head == NULL) return;
	if (L->head == L->tail) {
		free(L->head);
		L->head = NULL;
		L->tail = NULL;
	}
	else {
		pNode temp = L->head;
		while (temp->next != L->tail) temp = temp->next;
		free(temp->next);
		temp->next = NULL;
		L->tail = temp;
	}
	(L->count)--;
}

// 앞뒤 공백을 지우고, 연속된 공백은 하나로 줄인다. (postfix 결과 정리용)
void removeSpace(pList L) {
	while (L->head != NULL && L->head->data == ' ') deleteHead(L);
	while (L->tail != NULL && L->tail->data == ' ') deleteTail(L);
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

// 모든 공백 노드를 제거한다. (check 통과 후 계산 전에 호출)
void removeAllSpace(pList L) {
	pNode prev = NULL;
	pNode cur = L->head;
	while (cur != NULL) {
		pNode next = cur->next;
		if (cur->data == ' ') {
			if (prev == NULL) L->head = next;
			else prev->next = next;
			if (cur == L->tail) L->tail = prev;
			free(cur);
			(L->count)--;
		}
		else {
			prev = cur;
		}
		cur = next;
	}
}

// 리스트의 글자를 차례로 출력한다.
void printList(pList L) {
	if (L->head == NULL) { printf("\n"); return; }
	pNode tmp = L->head;
	while (tmp != NULL) {
		printf("%c", tmp->data);
		tmp = tmp->next;
	}
	printf("\n");
}

// 모든 노드의 메모리를 해제한다. (count 에 의존하지 않고 NULL 까지 순회)
void freeList(pList L) {
	pNode tmp = L->head;
	while (tmp != NULL) {
		pNode next = tmp->next;
		free(tmp);
		tmp = next;
	}
	L->head = NULL;
	L->tail = NULL;
	L->count = 0;
}
