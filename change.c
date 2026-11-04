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
	pNode curr = expr->head;
	// -1   3+(-1)
	int count = 1;
	while (curr != NULL) {
		if(curr == expr->head && ( curr->data == '-' || curr->data == '+')) {
			insertFront(expr,makeNode('0'));
		}
		else if(curr->next != NULL && curr->data=='(' && (curr->next->data=='-' || curr->next->data == '+')) {
			insertMid(expr,count,makeNode('0'));
		}
		curr = curr->next;
		count++;
	}
}
