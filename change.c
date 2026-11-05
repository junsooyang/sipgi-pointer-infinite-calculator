#include "calc.h"

void change(pList expr) {
	// 괄호 사이 생략된 곱셈 처리: 3(4) -> 3*(4), (2)(3) -> (2)*(3), (2)3 -> (2)*3
	pNode cur = expr->head;
	while (cur != NULL && cur->next != NULL) {
		char c = cur->data;
		char n = cur->next->data;
		if (((isdigit(c) || c == ')') && n == '(') || (c == ')' && isdigit(n))) {
			pNode star = makeNode('*');
			star->next = cur->next;
			cur->next = star;
			(expr->count)++;
			cur = star;
		}
		cur = cur->next;
	}
	// 단항 부호 처리: 맨 앞의 -1 -> 0-1, 괄호 바로 뒤의 (-1 -> (0-1
	// 번호로 세면 삽입할 때마다 위치가 밀려 어긋나므로, 보고 있는 노드 바로 뒤에 0을 끼운다.
	pNode curr = expr->head;
	// 맨 앞이 부호면 리스트 맨 앞에 0을 넣는다 (0 뒤부터 다시 순회)
	if (curr != NULL && (curr->data == '-' || curr->data == '+')) {
		insertFront(expr, makeNode('0'));
		curr = expr->head; // 새로 들어간 '0'
	}
	// '(' 바로 뒤가 부호면 그 '(' 다음에 0을 끼운다
	while (curr != NULL) {
		if (curr->data == '(' && curr->next != NULL &&
		    (curr->next->data == '-' || curr->next->data == '+')) {
			pNode zero = makeNode('0');
			zero->next = curr->next;
			curr->next = zero;
			(expr->count)++;
		}
		curr = curr->next;
	}
}
