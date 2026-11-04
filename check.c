#include <stdio.h>
#include <string.h>
#include "calc.h"

int check(pList expr){
	int dot = 0;
	int space = 0;
	int operator = 0;
	int left = 0;
	int right = 0;
	int count = 0;
	char prev = 0; // 직전 문자
	pNode curr = expr->head;
	if (curr == NULL) return 0; // 빈 입력
	if (curr->data == '.' || curr->data == ' ' || curr->data == '*' || curr->data == '/') return 0; 
	while(curr != NULL){
		count++;
		// 앞자리 0 금지: 숫자의 첫 글자가 0인데 바로 뒤에 숫자가 오면 에러 (029 불가, 0.5 / 10.05 / 100 은 허용)
		if (curr->data == '0' && !isdigit(prev) && prev != '.' && curr->next != NULL && isdigit(curr->next->data)) return 0;
		if (!(isdigit(curr->data) || curr->data == ' ' || curr->data == '.' || curr->data == '(' || curr->data == ')' || curr->data == '+' || curr->data == '-' || curr->data == '*' || curr->data == '/')) return 0;
		if (curr->data == '.'){ // .. , . 
			dot++;
			if (!isdigit(prev)) return 0; // 소수점 앞에는 숫자가 있어야 함 (1+.5, (.5) 불가)
			if (curr->next == NULL || !isdigit(curr->next->data)) return 0;
			if (dot >= 2) return 0;
		}
		if (curr->data == ' '){  //    ,  (
			dot = 0;
			space++;
			if (curr->next != NULL && (curr->next->data == ')' || curr->next->data == '.' )) return 0;
			if (space >= 2) return 0;
		}
		if (isdigit(curr->data) ){ //123 ( , 123 123
			if (curr->next != NULL && curr->next->data == ' '){
				if (curr->next->next != NULL && (curr->next->next->data == '(' || curr->next->next->data == ')' || isdigit(curr->next->next->data))) return 0;
			}
		}
		if (curr->data == '0'){
			if (curr->next != NULL && curr->next->data == '.'){
				if (curr->next->next != NULL && (!isdigit(curr->next->next->data))) return 0;
			}
		}
		if (curr->data == '('){
			dot = 0;
			left++;
			// 빈 괄호 금지: ()
			if (curr->next != NULL && curr->next->data == ')') return 0;
		}
		if (curr->data == ')'){
			dot = 0;
			right++;
			if (right >left) return 0;
		}
		if (curr->data == '+' || curr->data == '-' || curr->data == '*' || curr->data == '/'){
			operator++;
			dot = 0;
			if (operator >= 2) return 0;
			if (curr->next != NULL && curr->next->data == ')') return 0; // (1+) 불가
			if (curr->next != NULL && curr->next->data == ' '){
				if (curr->next->next != NULL && (curr->next->next->data == '+' || curr->next->next->data == '-' || curr->next->next->data == '*' || curr->next->next->data == '/')) return 0;
			}
		}
		if (curr->data == '(')
			if (curr->next != NULL && (curr->next->data == '*' || curr->next->data == '/')) return 0;
		if (curr->data != ' ') space = 0;
		if (!(curr->data == '+' || curr->data == '-' || curr->data == '*' || curr->data == '/')) operator = 0;
		prev = curr->data;
    	curr = curr->next;
   }
	if (left != right) return 0;
	// 끝의 공백은 무시하고, 마지막 의미 있는 문자가 연산자면 에러 (5+)
	if (prev == ' ') {
		pNode t = expr->head;
		char lastNonSpace = 0;
		while (t != NULL) {
			if (t->data != ' ') lastNonSpace = t->data;
			t = t->next;
		}
		prev = lastNonSpace;
	}
	if (prev == '+' || prev == '-' || prev == '*' || prev == '/') return 0;
	return 1;
}
/*
int main(void) {
	char expr[] = "120.123-10.12";
	pList list = makeList();
	for (int i =0; i<strlen(expr);i++) {
		insertBack(list, makeNode(expr[i]));
	}
	printf("%d \n",check(list));
	return 0;
}
*/
