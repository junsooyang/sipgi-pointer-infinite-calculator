#include <stdio.h>
#include <string.h>
#include "calc.h"

int check(pList expr){
	int dot = 0;
	int operator = 0;
	int left = 0;
	int right = 0;
	char prev = 0; // 직전 비공백 문자
	pNode curr = expr->head;
	if (curr == NULL) return 0; // 빈 입력
	// 맨 앞 공백은 허용(뒤에서 제거). 공백을 건너뛴 첫 실제 문자가 . * / 면 잘못된 식.
	{
		pNode f = curr;
		while (f != NULL && f->data == ' ') f = f->next;
		if (f == NULL) return 0; // 공백뿐인 입력
		if (f->data == '.' || f->data == '*' || f->data == '/') return 0;
	}
	while(curr != NULL){
		// 앞자리 0 금지: 숫자의 첫 글자가 0인데 바로 뒤에 숫자가 오면 에러 (029 불가, 0.5 / 10.05 / 100 은 허용)
		if (curr->data == '0' && !isdigit(prev) && prev != '.' && curr->next != NULL && isdigit(curr->next->data)) return 0;
		if (!(isdigit(curr->data) || curr->data == ' ' || curr->data == '.' || curr->data == '(' || curr->data == ')' || curr->data == '+' || curr->data == '-' || curr->data == '*' || curr->data == '/')) return 0;
		if (curr->data == '.'){ // .. , . 
			dot++;
			if (!isdigit(prev)) return 0; // 소수점 앞에는 숫자가 있어야 함 (1+.5, (.5) 불가)
			if (curr->next == NULL || !isdigit(curr->next->data)) return 0;
			if (dot >= 2) return 0;
		}
		if (curr->data == ' '){
			dot = 0;
			// 숫자와 숫자 사이 공백만 금지 ("1 2" 불가). 나머지 공백은 뒤에서 제거하므로 허용.
			if (isdigit(prev)) {
				pNode q = curr->next;
				while (q != NULL && q->data == ' ') q = q->next; // 연속 공백 건너뛰기
				if (q != NULL && (isdigit(q->data) || q->data == '.')) return 0;
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
			// 빈 괄호 금지: () , ( )
			pNode q = curr->next;
			while (q != NULL && q->data == ' ') q = q->next;
			if (q != NULL && q->data == ')') return 0;
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
			// 뒤따르는 공백을 건너뛴 다음 문자 확인
			pNode q = curr->next;
			while (q != NULL && q->data == ' ') q = q->next;
			if (q != NULL && q->data == ')') return 0; // (1+) 불가
			if (q != NULL && (q->data == '+' || q->data == '-' || q->data == '*' || q->data == '/')) return 0; // 연산자 연속
		}
		if (curr->data == '(') {
			pNode q = curr->next;
			while (q != NULL && q->data == ' ') q = q->next;
			if (q != NULL && (q->data == '*' || q->data == '/')) return 0; // (* , (/
		}
		if (curr->data != ' ') prev = curr->data; // prev 는 직전 비공백 문자
		if (!(curr->data == '+' || curr->data == '-' || curr->data == '*' || curr->data == '/')) operator = 0;
    	curr = curr->next;
   }
	if (left != right) return 0;
	// prev 는 직전 비공백 문자이므로, 식이 연산자로 끝나면(뒤에 공백이 있어도) 에러 (5+ , 5+ )
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
