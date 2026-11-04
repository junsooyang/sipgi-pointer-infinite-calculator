#include "calc.h"

/* 연산자 우선순위: * / 가 + - 보다 높다. '(' 는 0 */
static int prec(int op) {
	if (op == '*' || op == '/') return 2;
	if (op == '+' || op == '-') return 1;
	return 0;
}

/* 중위표기식 -> 후위표기식 (숫자와 연산자는 공백 하나로 구분) */
pList postfix(pList infix)
{
	pList list = makeList();
	struct node* head = NULL; // 연산자 스택
	pNode start = infix->head;

	while (start != NULL) {
		char c = start->data;
		if (isdigit(c) || c == '.') {
			insertBack(list, makeNode(c));
		}
		else if (c == '+' || c == '-' || c == '*' || c == '/') {
			insertBack(list, makeNode(' '));
			/* 스택 top 의 우선순위가 같거나 높으면 먼저 꺼낸다 (왼쪽 결합) */
			while (!empty(head) && prec(top(head)) >= prec(c)) {
				insertBack(list, makeNode((char)pop(&head)));
				insertBack(list, makeNode(' '));
			}
			push(&head, c);
		}
		else if (c == '(') {
			insertBack(list, makeNode(' '));
			push(&head, c);
		}
		else if (c == ')') {
			insertBack(list, makeNode(' '));
			while (!empty(head) && top(head) != '(') {
				insertBack(list, makeNode((char)pop(&head)));
				insertBack(list, makeNode(' '));
			}
			if (!empty(head)) pop(&head); // '(' 제거
		}
		start = start->next;
	}
	insertBack(list, makeNode(' '));
	while (!empty(head)) {
		insertBack(list, makeNode((char)pop(&head)));
		insertBack(list, makeNode(' '));
	}
	removeSpace(list);
	return list;
}
