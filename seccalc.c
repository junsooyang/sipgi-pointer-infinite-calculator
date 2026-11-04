#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include "calc.h"

int power(int a, int b) {
	double ans = 1;
	while (b > 0) {
		ans *= a;
		b--;
	}
	return ans;
}

pList add(pList,pList);
pList minus(pList a, pList b);

/* 리스트의 자릿수(0~9) 문자들을 정수 배열(낮은 자리부터)로 옮긴다.
   '.' 이 없는 순수 정수 리스트여야 한다. 반환값은 malloc 된 배열, 길이는 *len. */
static int* listToDigits(pList L, int* len) {
	int n = 0;
	pNode p = L->head;
	while (p != NULL) {
		if (p->data != '.') n++;
		p = p->next;
	}
	int* arr = (int*)malloc(sizeof(int) * (n > 0 ? n : 1));
	/* L 은 높은 자리 → 낮은 자리 순. 배열은 낮은 자리(인덱스0)부터 저장 ('.' 은 건너뜀) */
	int i = n - 1;
	p = L->head;
	while (p != NULL) {
		if (p->data != '.') {
			arr[i] = p->data - '0';
			i--;
		}
		p = p->next;
	}
	*len = n;
	return arr;
}

/* 양수 정수 리스트 a, b 를 곱해 순수 정수 결과 리스트로 반환 (세로셈) */
static pList mulDigits(pList a, pList b) {
	int la, lb;
	int* da = listToDigits(a, &la); // 낮은 자리부터
	int* db = listToDigits(b, &lb);

	int lr = la + lb;
	int* res = (int*)calloc(lr, sizeof(int)); // 결과 자리 (낮은 자리부터)

	for (int i = 0; i < la; i++) {
		for (int j = 0; j < lb; j++) {
			res[i + j] += da[i] * db[j];
		}
	}
	/* 받아올림 정리 (la+lb 자리이므로 최상위 자리에서 넘침 없음) */
	for (int k = 0; k < lr - 1; k++) {
		if (res[k] >= 10) {
			res[k + 1] += res[k] / 10;
			res[k] %= 10;
		}
	}
	/* 높은 자리 → 낮은 자리 순으로 리스트 구성 */
	pList out = makeList();
	int start = lr - 1;
	while (start > 0 && res[start] == 0) start--; // 선행 0 제거
	for (int k = start; k >= 0; k--) {
		insertBack(out, makeNode(res[k] + '0'));
	}
	if (out->head == NULL) insertBack(out, makeNode('0'));

	free(da); free(db); free(res);
	return out;
}

pList multiply(pList a, pList b) {
	int one, two;
	one = (a->head->data != '-' ? 1 : 0);
	two = (b->head->data != '-' ? 1 : 0);

	if (one == 1 && two == 1) {
		/* 소수부 길이 구하고 '.' 제거 → 순수 정수로 만든다 */
		int a_dec = 0, b_dec = 0;
		if (isNode(a, '.')) a_dec = a->count - searchNode(a, '.');
		if (isNode(b, '.')) b_dec = b->count - searchNode(b, '.');
		int dotPos = a_dec + b_dec; // 결과 소수부 길이

		pList sum = mulDigits(a, b); // 순수 정수 곱

		if (dotPos > 0) {
			/* 소수 결과: 뒤에서 dotPos 자리 앞에 '.' 삽입 */
			/* 정수부가 모자라면 앞에 0 을 채워 '.' 을 넣을 자리를 확보 */
			while (sum->count <= dotPos) insertFront(sum, makeNode('0'));
			insertMid(sum, sum->count - dotPos, makeNode('.'));
			/* 불필요한 0 정리 (소수라 앞/뒤 0 제거 모두 안전) */
			removeZero(sum);
			if (sum->head == NULL) insertBack(sum, makeNode('0'));
			if (sum->head->data == '.') insertFront(sum, makeNode('0'));
			if (sum->tail->data == '.') deleteTail(sum);
		}
		else {
			/* 정수 결과: 앞쪽 0 만 제거 (뒤쪽 0 은 유효 자릿수) */
			while (sum->count > 1 && sum->head->data == '0') deleteHead(sum);
		}

		return sum;
	}
	else if (one == 1 && two == 0) { // a * (-b)
		deleteHead(b);
		pList answer = multiply(a, b);
		if (!(answer->count == 1 && answer->head->data == '0'))
			insertFront(answer, makeNode('-'));
		return answer;
	}
	else if (one == 0 && two == 1) { // (-a) * b
		deleteHead(a);
		pList answer = multiply(a, b);
		if (!(answer->count == 1 && answer->head->data == '0'))
			insertFront(answer, makeNode('-'));
		return answer;
	}
	else { // (-a) * (-b)
		deleteHead(a);
		deleteHead(b);
		return multiply(a, b);
	}
}

pList add(pList a , pList b ) {

	int one,two;

	one = (a-> head -> data != '-'?1:0);

	two = (b-> head -> data != '-'?1:0);

	if ( one == 1 && two == 1){
		if (isNode(a, '.') == 0){
			insertBack(a, makeNode('.'));
			insertBack(a, makeNode('0'));
		}
		if (isNode(b, '.') == 0){
			insertBack(b, makeNode('.'));
			insertBack(b,makeNode('0'));
		}
	// 자연수일 경우 . 추가
		int a_count = a->count;
		int b_count = b->count;
   		int a_point = searchNode(a, '.');
   		int b_point = searchNode(b, '.');
   		int a_int_len = a_point - 1;
   		int b_int_len = b_point - 1;
   		int a_dec_len = a_count - a_point;
   		int b_dec_len = b_count - b_point;
		if (a_int_len != b_int_len) {
   			while ( a_int_len != b_int_len) // int 자리수 맞추기
			{
	   			if (a_int_len > b_int_len) {
		   			insertFront(b, makeNode('0'));
		   			b_int_len++;
				}
	   			else if (a_int_len < b_int_len) {
		   			insertFront(a, makeNode('0'));
		   			a_int_len++;
				}
			}
		}
		if (a_dec_len != b_dec_len) {
   			while (a_dec_len != b_dec_len) // dec 자리수 맞추기
			{
	   			if (a_dec_len > b_dec_len) {
		   			insertBack(b, makeNode('0'));
		   			b_dec_len++;
				}
	   			else if (a_dec_len < b_dec_len) {
		   			insertBack(a, makeNode('0'));
		   			a_dec_len++;
				}	
			}
		}
		 // 999.9990 + 009.9999
		pList pst = makeList();
		pList answer = makeList();
		// 123.40   001.2
		while (a->head != NULL){
			if ((a->head->data - '0' + b->head->data - '0') > 9) {
				insertBack(pst, makeNode('1'));
				insertBack(answer, makeNode(a->head->data - '0' + b->head->data - '0' -10 + '0'));
				deleteHead(a);
				deleteHead(b);
			}
			else if ((a->head->data - '0' + b->head->data - '0')  < 10 && a->head->data - '0' + b->head->data - '0' > 0) {
				insertBack(pst , makeNode('0'));
				insertBack(answer, makeNode(a->head->data -'0' + b->head->data-'0'+'0'));
				deleteHead(a);
				deleteHead(b);
			}
			else if ((a->head->data - '0' + b->head->data -'0') == 0){
				insertBack(pst, makeNode('0'));
				insertBack(answer, makeNode('0'));
				deleteHead(a);
				deleteHead(b);
			}
			else if ((a->head->data - '0' +b->head->data - '0') == 10){
				insertBack(pst, makeNode('1'));
				insertBack(answer, makeNode('0'));
				deleteHead(a);
				deleteHead(b);
			}
			else if (a->head->data == '.' && b->head->data  == '.') {
				insertBack(answer, makeNode('.'));
				deleteHead(a);
				deleteHead(b);
			}
		} 
		insertBack(pst, makeNode('0'));
		int answer_Pointer = searchNode(answer,'.');
		insertMid(pst, answer_Pointer, makeNode('.'));
		// . 삽입 clea	
		if (isNode(pst, '1') == 0) {
			removeZero(answer);
			if(answer->head->data == '.') insertFront(answer,makeNode('0'));
		if(answer->tail->data == '.') deleteTail(answer);
			return answer;
		}
		else if (isNode(pst, '1') == 1) {
			return add(answer, pst);
		}
	}

	else if ( one == 1 && two == 0) { // add( a,-b )
		deleteHead(b);
		return minus(a,b);
	}
	else if ( one == 0 && two == 1) { // add(-a,b)
		deleteHead(a);
		return minus(b,a);
	}

	else if ( one == 0 && two == 0) { // add(-a,-b)
		deleteHead(a);
		deleteHead(b);
		pList ans = add(a,b);
		insertFront(ans,makeNode('-'));
		return ans;	
	}
}
pList minus(pList a , pList b ) {

	int one,two;
	one = (a-> head -> data != '-'?1:0);
	two = (b-> head -> data != '-'?1:0);
	char a_head = (a -> head -> data);
	char b_head = (b -> head -> data);

	if ( one == 1 && two == 1) {
		if (isNode(a, '.') == 0){
			insertBack(a, makeNode('.'));
			insertBack(a, makeNode('0'));
		}
		if(isNode(b, '.') == 0){
			insertBack(b, makeNode('.'));
			insertBack(b, makeNode('0'));
		}
		// 자연수일 경우 뒤에 . 추가
		int a_count = a->count;
		int b_count = b->count;
		int a_point = searchNode(a, '.');
		int b_point = searchNode(b, '.');
		int a_int_len = a_point -1;
		int b_int_len = b_point -1;
		int a_dec_len = a_count - a_point;
		int b_dec_len = b_count - b_point;
		while (a_int_len != b_int_len) {
			if (a_int_len > b_int_len) {
				insertFront(b, makeNode('0'));
				b_int_len++;
			}	
			else if (a_int_len < b_int_len) {
				insertFront(a, makeNode('0'));
				a_int_len++;
			}
		}
		while (a_dec_len != b_dec_len) {
			if (a_dec_len > b_dec_len) {
				insertBack(b, makeNode('0'));
				b_dec_len++;
			}
			else if (a_dec_len < b_dec_len) {
				insertBack(a, makeNode('0'));
				a_dec_len++;
			}
		}
		if (compareList(a,b) == 0) {
			pList ans = minus(b,a);
			insertFront(ans,makeNode('-'));
			return ans;

		}
		pList pst = makeList();
		pList answer = makeList();
		while (a->head != NULL) {
			if (a->head->data == '.' && b->head->data == '.'){
				insertBack(answer, makeNode('.'));
				deleteHead(a);
				deleteHead(b);
			}
			else if (a->head->data - b->head->data < 0) {
				insertBack(pst,makeNode('1'));
				insertBack(answer, makeNode( 10 - (b->head->data - a->head->data) + '0'));
				deleteHead(a);
				deleteHead(b);
			}
			else if (a->head->data - b->head->data > 0) {
				insertBack(pst, makeNode('0'));
				insertBack(answer, makeNode(a->head->data - b->head->data +'0'));
				deleteHead(a);
				deleteHead(b);
			}
			else if (a->head->data != '.' && b->head->data != '.' && a->head->data - b->head->data == 0) {
				insertBack(pst, makeNode('0'));
				insertBack(answer, makeNode('0'));
				deleteHead(a);
				deleteHead(b);
				}
		}	
		insertBack(pst, makeNode('0'));
		int answer_Pointer = searchNode(answer, '.');
		insertMid(pst,answer_Pointer,makeNode('.'));
		if (isNode(pst, '1') == 0) {
			removeZero(answer);
			if(answer->head->data == '.') insertFront(answer,makeNode('0'));
			if(answer->tail->data == '.') deleteTail(answer);
			return answer;
		}
		else if (isNode(pst, '1') == 1) {
			return minus(answer, pst);
		}
	}
	else if ( one == 1 && two == 0) { // minus( a,-b )
		deleteHead(b);
		return add(a, b);
	}		
	else if ( one == 0 && two == 1) { // minus(-a,b)
		deleteHead(a);
		pList ans = add(a,b);
		insertFront(ans,makeNode('-'));
		return ans;
	}
	else if ( one == 0 && two == 0) { // minus( -a , +b )
		deleteHead(b);
		return minus(a, b);
	}
}

/* 예전 pList 기반 계산기 (나눗셈 미지원). 지금은 rational.c 의 seccalc 를 사용한다. */
pList seccalc_legacy(pList expr) {
	pNode curr = expr->head;
	struct nodeL* head = NULL; //make stack
	int word = 0;
	pList tempList,a,b;
	while(curr != NULL) {
		pList temp;
		if((isdigit(curr->data) || curr->data == '.') && word == 0) {
			tempList = makeList();
			insertBack(tempList, makeNode(curr->data));
			word = 1;
		}
		else if((isdigit(curr->data) || curr->data == '.') && word == 1) {
			insertBack(tempList, makeNode(curr->data));
		}
		else if(isspace(curr->data)) {
			if(word == 1)
				pushL(&head,tempList);//stack push (숫자를 읽은 직후에만)
			word = 0;
		}
		else if(curr->data == '+') {
			a = popL(&head); //pop
			b = popL(&head); //pop
			temp = add(a,b);
			pushL(&head, temp);//push temp
		}
		else if(curr->data == '-') {
			b = popL(&head); //pop
			a = popL(&head); //pop
			temp = minus(a,b);
			pushL(&head, temp);//push temp
		}
		else if(curr->data == '*') {
			a = popL(&head); //pop
			b = popL(&head); //pop
			temp = multiply(a,b);
			pushL(&head, temp); //push temp
		}
		curr = curr->next;
	}
	pList ans = topL(head); //stack top
	return ans;
}/*
int main(void) {
	char expr1[] = "8";
	char expr2[] = "8";
	pList a = makeList();
	pList b = makeList();
	for (int i = 0; i<strlen(expr1);i++) {
		insertBack(a,makeNode(expr1[i]));
	}
	for(int i = 0; i<strlen(expr2);i++) {
		insertBack(b,makeNode(expr2[i]));
	}
	printList(a);
	printList(b);
	pList ans = add(a,b);
	printList(ans);
	freeList(a);
	freeList(b);
	freeList(ans);
	free(a);
	free(b);
	free(ans);
	return 0;
}*/
