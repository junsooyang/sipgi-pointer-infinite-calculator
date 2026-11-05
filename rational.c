#include "calc.h"

/*
 * 분수(유리수) 기반 무한 정밀도 계산
 *  - 모든 값을 부호 / 분자 / 분모(큰 자연수)로 저장해서 + - * / 를 오차 없이 계산한다.
 *  - 출력할 때만 소수로 바꾸고, 끝나지 않는 소수는 순환마디를 괄호로 표시한다.
 *      1/3 -> 0.(3),  1/6 -> 0.1(6),  1/7 -> 0.(142857)
 *  - 순환마디가 MAX_PERIOD 자리를 넘으면 그만큼만 출력하고 "..." 을 붙인다.
 *
 * 성능 관련 설계
 *  1) 큰 자연수는 10000진법(배열 칸 하나에 10진 4자리)으로 저장한다.
 *  2) 나눗셈/소수 변환에서 자리마다 malloc 하지 않고 버퍼를 재사용한다 (용량을 2배씩 늘림).
 *  3) 덧셈/뺄셈은 분모의 gcd 로 공통분모를 최소로 맞추고, 곱셈/나눗셈은 미리 교차 약분한다.
 *  4) 긴 나눗셈은 몫 자리를 부동소수로 추정한 뒤 보정하고, 나뉘는 수의 상위 자리는 한 번에
 *     복사해서 O((la-lb+1)*lb) 로 끝낸다. 그래서 유클리드 gcd 전체 비용이 O(n^2) 이 된다.
 */

int calcError = 0;

#define MAX_PERIOD 10000
#define BASE 10000      // 배열 칸 하나 = 10진 4자리

/* ---------- 큰 자연수 (d[0] 이 가장 낮은 칸, n==0 이면 0) ---------- */
typedef struct { int *d; int n; int cap; } Nat;

static void natReserve(Nat *a, int cap) {
	if (cap <= a->cap) return;
	int nc = a->cap > 0 ? a->cap : 4;
	while (nc < cap) nc *= 2;
	a->d = (int*)realloc(a->d, sizeof(int) * nc);
	a->cap = nc;
}

static Nat natNew(int cap) {
	Nat a;
	a.d = NULL; a.n = 0; a.cap = 0;
	natReserve(&a, cap > 0 ? cap : 1);
	return a;
}

static void natFree(Nat *a) { free(a->d); a->d = NULL; a->n = 0; a->cap = 0; }
static void natTrim(Nat *a) { while (a->n > 0 && a->d[a->n - 1] == 0) a->n--; }
static int natIsZero(const Nat *a) { return a->n == 0; }
static int natIsOne(const Nat *a) { return a->n == 1 && a->d[0] == 1; }

static Nat natFromSmall(int v) {
	Nat a = natNew(2);
	while (v > 0) { a.d[a.n++] = v % BASE; v /= BASE; }
	return a;
}

static void natAssign(Nat *dst, const Nat *src) {
	natReserve(dst, src->n);
	if (src->n > 0) memcpy(dst->d, src->d, sizeof(int) * src->n);
	dst->n = src->n;
}

static Nat natCopy(const Nat *a) {
	Nat b = natNew(a->n);
	natAssign(&b, a);
	return b;
}

static int natCmp(const Nat *a, const Nat *b) {
	if (a->n != b->n) return a->n > b->n ? 1 : -1;
	for (int i = a->n - 1; i >= 0; i--)
		if (a->d[i] != b->d[i]) return a->d[i] > b->d[i] ? 1 : -1;
	return 0;
}

static Nat natAdd(const Nat *a, const Nat *b) {
	int n = (a->n > b->n ? a->n : b->n) + 1;
	Nat c = natNew(n);
	int carry = 0;
	for (int i = 0; i < n; i++) {
		int s = carry + (i < a->n ? a->d[i] : 0) + (i < b->n ? b->d[i] : 0);
		c.d[i] = s % BASE;
		carry = s / BASE;
	}
	c.n = n;
	natTrim(&c);
	return c;
}

/* a -= b  (a >= b 이어야 함) */
static void natSubInPlace(Nat *a, const Nat *b) {
	int borrow = 0;
	for (int i = 0; i < a->n; i++) {
		int s = a->d[i] - borrow - (i < b->n ? b->d[i] : 0);
		if (s < 0) { s += BASE; borrow = 1; }
		else borrow = 0;
		a->d[i] = s;
		if (i >= b->n && borrow == 0) break; // 더 뺄 게 없으면 조기 종료
	}
	natTrim(a);
}

static Nat natSub(const Nat *a, const Nat *b) {
	Nat c = natCopy(a);
	natSubInPlace(&c, b);
	return c;
}

static Nat natMul(const Nat *a, const Nat *b) {
	if (natIsZero(a) || natIsZero(b)) return natNew(1);
	int n = a->n + b->n;
	/* 칸 곱은 1e8 미만이라 long long 에 모았다가 마지막에 한 번만 올림 처리 */
	long long *t = (long long*)calloc(n, sizeof(long long));
	for (int i = 0; i < a->n; i++) {
		long long ai = a->d[i];
		if (ai != 0) {
			for (int j = 0; j < b->n; j++) t[i + j] += ai * b->d[j];
		}
		/* 주기적으로 올림을 정리해 long long 오버플로를 막는다.
		   이 정리는 ai 값과 무관하게 항상 수행해야 안전하다. */
		if ((i & 1023) == 1023) {
			for (int k = 0; k < n - 1; k++) { t[k + 1] += t[k] / BASE; t[k] %= BASE; }
		}
	}
	for (int k = 0; k < n - 1; k++) { t[k + 1] += t[k] / BASE; t[k] %= BASE; }
	Nat c = natNew(n);
	for (int k = 0; k < n; k++) c.d[k] = (int)t[k];
	c.n = n;
	natTrim(&c);
	free(t);
	return c;
}

/* dst = a * s  (0 <= s < BASE) */
static void natMulSmallInto(Nat *dst, const Nat *a, int s) {
	natReserve(dst, a->n + 1);
	int carry = 0;
	for (int i = 0; i < a->n; i++) {
		int v = a->d[i] * s + carry;
		dst->d[i] = v % BASE;
		carry = v / BASE;
	}
	dst->n = a->n;
	if (carry) dst->d[dst->n++] = carry;
	natTrim(dst);
}

/* a *= s  (0 <= s < BASE) */
static void natMulSmallInPlace(Nat *a, int s) {
	int carry = 0;
	for (int i = 0; i < a->n; i++) {
		int v = a->d[i] * s + carry;
		a->d[i] = v % BASE;
		carry = v / BASE;
	}
	if (carry) {
		natReserve(a, a->n + 1);
		a->d[a->n++] = carry;
	}
	natTrim(a);
}

/* a = a*BASE + limb  (버퍼 재사용, malloc 없음) */
static void natShiftAdd(Nat *a, int limb) {
	if (a->n == 0) {
		if (limb) { natReserve(a, 1); a->d[0] = limb; a->n = 1; }
		return;
	}
	natReserve(a, a->n + 1);
	memmove(a->d + 1, a->d, sizeof(int) * a->n);
	a->d[0] = limb;
	a->n++;
}

/* a /= s (작은 정수), 나머지 반환 */
static int natDivSmallInPlace(Nat *a, int s) {
	int rem = 0;
	for (int i = a->n - 1; i >= 0; i--) {
		int cur = rem * BASE + a->d[i];
		a->d[i] = cur / s;
		rem = cur % s;
	}
	natTrim(a);
	return rem;
}

/* 상위 k 칸의 근삿값 (실제 값 ~= 반환값 * BASE^exp) */
static double natLead(const Nat *a, int k, int *exp) {
	if (k > a->n) k = a->n;
	double v = 0;
	for (int i = a->n - 1; i >= a->n - k; i--) v = v * BASE + a->d[i];
	*exp = a->n - k;
	return v;
}

/*
 * R < b*BASE 일 때 q = floor(R/b) (0 ~ BASE-1) 를 구하고 R -= q*b.
 * 상위 칸으로 q 를 추정한 뒤 위/아래로 보정하므로 결과는 항상 정확하다.
 */
static int natDivStep(Nat *R, const Nat *b, Nat *tmp) {
	if (natCmp(R, b) < 0) return 0;
	int er, eb;
	double rv = natLead(R, 3, &er);
	double bv = natLead(b, 2, &eb);
	double qd = rv / bv;
	for (int e = er - eb; e > 0; e--) qd *= BASE;
	for (int e = er - eb; e < 0; e++) qd /= BASE;
	long long q = (long long)qd;
	if (q > BASE - 1) q = BASE - 1;
	if (q < 0) q = 0;

	natMulSmallInto(tmp, b, (int)q);
	while (natCmp(tmp, R) > 0) { q--; natSubInPlace(tmp, b); } // 추정이 컸으면 내림
	natSubInPlace(R, tmp);
	while (natCmp(R, b) >= 0) { natSubInPlace(R, b); q++; }    // 추정이 작았으면 올림
	return (int)q;
}

/* 긴 나눗셈: q = a / b, r = a % b  (b != 0) */
static void natDivMod(const Nat *a, const Nat *b, Nat *q, Nat *r) {
	Nat Q = natNew(a->n + 1);
	Nat R = natNew(b->n + 2);
	if (natCmp(a, b) < 0) {
		natAssign(&R, a);
	}
	else {
		Nat tmp = natNew(b->n + 2);
		/* 나뉘는 수의 상위 (lb-1) 칸은 어차피 몫이 0 이라 한 번에 복사 */
		int start = a->n - (b->n - 1);
		R.n = b->n - 1;
		if (R.n > 0) memcpy(R.d, a->d + start, sizeof(int) * R.n);
		natTrim(&R);
		Q.n = start;
		for (int i = start - 1; i >= 0; i--) {
			natShiftAdd(&R, a->d[i]);
			Q.d[i] = natDivStep(&R, b, &tmp);
		}
		natTrim(&Q);
		natFree(&tmp);
	}
	*q = Q;
	*r = R;
}

/* x /= g  (나누어떨어지는 경우에만 사용) */
static void natDivExact(Nat *x, const Nat *g) {
	if (natIsOne(g)) return;
	Nat q, r;
	natDivMod(x, g, &q, &r);
	natFree(&r);
	natFree(x);
	*x = q;
}

static Nat natGcd(const Nat *a, const Nat *b) {
	Nat x = natCopy(a), y = natCopy(b);
	while (!natIsZero(&y)) {
		Nat q, r;
		natDivMod(&x, &y, &q, &r);
		natFree(&q);
		natFree(&x);
		x = y;
		y = r;
	}
	natFree(&y);
	return x;
}

/* ---------- 유리수 ---------- */
typedef struct { int sign; Nat num; Nat den; } Rat; // sign: 1 또는 -1, den > 0, 항상 기약분수

static void ratFree(Rat *r) { natFree(&r->num); natFree(&r->den); }

static Rat ratZero(void) {
	Rat c;
	c.sign = 1;
	c.num = natNew(1);
	c.den = natFromSmall(1);
	return c;
}

/* 분자가 0 이면 +0/1 로 맞춘다 */
static void ratFixZero(Rat *r) {
	if (natIsZero(&r->num)) {
		natFree(&r->den);
		r->den = natFromSmall(1);
		r->sign = 1;
	}
}

/*
 * a ± b
 * g = gcd(a.den, b.den) 로 공통분모를 a.den * (b.den/g) 로 최소화하고,
 * 결과 약분은 분자와 g 의 gcd 만 보면 된다 (기약분수끼리의 성질).
 */
static Rat ratAddSub(const Rat *a, const Rat *b, int subtract) {
	int sb = subtract ? -b->sign : b->sign;
	Nat g = natGcd(&a->den, &b->den);
	Nat da = natCopy(&a->den), db = natCopy(&b->den);
	natDivExact(&da, &g);  // a.den / g
	natDivExact(&db, &g);  // b.den / g

	Nat x = natMul(&a->num, &db);
	Nat y = natMul(&b->num, &da);
	Rat c;
	c.den = natMul(&a->den, &db);
	if (a->sign == sb) {
		c.num = natAdd(&x, &y);
		c.sign = a->sign;
	}
	else if (natCmp(&x, &y) >= 0) {
		c.num = natSub(&x, &y);
		c.sign = a->sign;
	}
	else {
		c.num = natSub(&y, &x);
		c.sign = sb;
	}
	natFree(&x); natFree(&y); natFree(&da); natFree(&db);

	if (natIsZero(&c.num)) {
		ratFixZero(&c);
	}
	else if (!natIsOne(&g)) {
		Nat g2 = natGcd(&c.num, &g);
		natDivExact(&c.num, &g2);
		natDivExact(&c.den, &g2);
		natFree(&g2);
	}
	natFree(&g);
	return c;
}

/* (an/ad) * (bn/bd) : an-bd, bn-ad 를 미리 교차 약분하면 결과가 바로 기약분수 */
static Rat ratMulParts(int sign, const Nat *an, const Nat *ad, const Nat *bn, const Nat *bd) {
	if (natIsZero(an) || natIsZero(bn)) return ratZero();
	Nat g1 = natGcd(an, bd);
	Nat g2 = natGcd(bn, ad);
	Nat x1 = natCopy(an), y1 = natCopy(bd);
	Nat x2 = natCopy(bn), y2 = natCopy(ad);
	natDivExact(&x1, &g1); natDivExact(&y1, &g1);
	natDivExact(&x2, &g2); natDivExact(&y2, &g2);
	Rat c;
	c.sign = sign;
	c.num = natMul(&x1, &x2);
	c.den = natMul(&y2, &y1);
	natFree(&g1); natFree(&g2);
	natFree(&x1); natFree(&y1); natFree(&x2); natFree(&y2);
	return c;
}

static Rat ratMul(const Rat *a, const Rat *b) {
	return ratMulParts(a->sign * b->sign, &a->num, &a->den, &b->num, &b->den);
}

static Rat ratDiv(const Rat *a, const Rat *b) {
	if (natIsZero(&b->num)) { // 0 으로 나눔
		calcError = 1;
		return ratZero();
	}
	/* a / b = (a.num/a.den) * (b.den/b.num) */
	return ratMulParts(a->sign * b->sign, &a->num, &a->den, &b->den, &b->num);
}

/* "123.45" -> 12345/100 (기약분수) */
static Rat ratParse(const char *s) {
	int len = (int)strlen(s);
	int frac = 0, seenDot = 0, digits = 0;
	for (int i = 0; i < len; i++) {
		if (s[i] == '.') seenDot = 1;
		else { digits++; if (seenDot) frac++; }
	}
	Rat r;
	r.sign = 1;

	/* 뒤에서부터 4자리씩 묶어 칸에 넣는다 */
	r.num = natNew(digits / 4 + 1);
	int limb = 0, mul = 1;
	for (int i = len - 1; i >= 0; i--) {
		if (s[i] == '.') continue;
		limb += (s[i] - '0') * mul;
		mul *= 10;
		if (mul == BASE) { r.num.d[r.num.n++] = limb; limb = 0; mul = 1; }
	}
	if (mul != 1) r.num.d[r.num.n++] = limb;
	natTrim(&r.num);

	/* 분모 10^frac */
	r.den = natNew(frac / 4 + 1);
	r.den.n = frac / 4 + 1;
	for (int i = 0; i < r.den.n; i++) r.den.d[i] = 0;
	int p = 1;
	for (int i = 0; i < frac % 4; i++) p *= 10;
	r.den.d[frac / 4] = p;

	if (natIsZero(&r.num)) { ratFixZero(&r); return r; }
	Nat g = natGcd(&r.num, &r.den);
	natDivExact(&r.num, &g);
	natDivExact(&r.den, &g);
	natFree(&g);
	return r;
}

/* 칸 하나를 10진 문자로 출력 (pad 면 앞을 0 으로 채워 4자리) */
static void emitLimb(pList out, int v, int pad) {
	char buf[5];
	int k = 0;
	for (int i = 0; i < 4; i++) { buf[k++] = (char)('0' + v % 10); v /= 10; if (!pad && v == 0) break; }
	while (k > 0) insertBack(out, makeNode(buf[--k]));
}

/* 유리수 -> 소수 문자열 (순환마디는 괄호) */
static pList ratToList(const Rat *r) {
	pList out = makeList();
	if (r->sign < 0 && !natIsZero(&r->num)) insertBack(out, makeNode('-'));

	Nat q, rem;
	natDivMod(&r->num, &r->den, &q, &rem);
	if (natIsZero(&q)) insertBack(out, makeNode('0'));
	for (int i = q.n - 1; i >= 0; i--) emitLimb(out, q.d[i], i != q.n - 1);
	natFree(&q);

	if (natIsZero(&rem)) { natFree(&rem); return out; }
	insertBack(out, makeNode('.'));

	/* 분모 = 2^a * 5^b * m  ->  순환하지 않는 앞부분 길이 = max(a, b)
	   (BASE 가 10 의 거듭제곱이라 2, 5 로 나누어지는지는 가장 낮은 칸만 보면 된다) */
	Nat m = natCopy(&r->den);
	int a2 = 0, b5 = 0;
	while (m.d[0] % 2 == 0) { natDivSmallInPlace(&m, 2); a2++; }
	while (m.d[0] % 5 == 0) { natDivSmallInPlace(&m, 5); b5++; }
	int pre = a2 > b5 ? a2 : b5;
	natFree(&m);

	Nat tmp = natNew(r->den.n + 2);

	/* 다음 소수 자리 하나: rem*10 을 den 으로 나눈 몫 */
	for (int i = 0; i < pre && !natIsZero(&rem); i++) {
		natMulSmallInPlace(&rem, 10);
		int dg = natDivStep(&rem, &r->den, &tmp);
		insertBack(out, makeNode((char)('0' + dg)));
	}

	if (!natIsZero(&rem)) {
		/* 여기부터는 순수 순환: 나머지가 처음 값으로 돌아오면 한 주기 끝 */
		int pos = out->count; // 순환마디는 이 노드 바로 뒤부터 시작
		Nat start = natCopy(&rem);
		int count = 0;
		do {
			natMulSmallInPlace(&rem, 10);
			int dg = natDivStep(&rem, &r->den, &tmp);
			insertBack(out, makeNode((char)('0' + dg)));
			count++;
		} while (natCmp(&rem, &start) != 0 && count < MAX_PERIOD);

		if (natCmp(&rem, &start) == 0) {
			/* 순환마디를 괄호로 감싼다: 0.1(6) */
			insertMid(out, pos, makeNode('('));
			insertBack(out, makeNode(')'));
		}
		else { // 순환마디가 너무 길면 일부만 출력
			insertBack(out, makeNode('.'));
			insertBack(out, makeNode('.'));
			insertBack(out, makeNode('.'));
		}
		natFree(&start);
	}
	natFree(&tmp);
	natFree(&rem);
	return out;
}

/* 후위표기식(공백 구분)을 분수로 계산해 결과 문자열 리스트를 반환 */
pList seccalc(pList expr) {
	calcError = 0;
	Rat *st = (Rat*)malloc(sizeof(Rat) * (expr->count + 1));
	int sp = 0;
	char *tok = (char*)malloc(expr->count + 2);
	int tl = 0;
	pNode p = expr->head;

	for (;;) {
		char c = (p != NULL) ? p->data : ' ';
		if (c == ' ') {
			if (tl > 0) {
				tok[tl] = '\0';
				if (tl == 1 && (tok[0] == '+' || tok[0] == '-' || tok[0] == '*' || tok[0] == '/')) {
					if (sp < 2) { calcError = 2; break; }
					Rat b = st[--sp];
					Rat a = st[--sp];
					Rat res;
					if (tok[0] == '+') res = ratAddSub(&a, &b, 0);
					else if (tok[0] == '-') res = ratAddSub(&a, &b, 1);
					else if (tok[0] == '*') res = ratMul(&a, &b);
					else res = ratDiv(&a, &b);
					ratFree(&a); ratFree(&b);
					st[sp++] = res;
					if (calcError) break;
				}
				else {
					st[sp++] = ratParse(tok);
				}
				tl = 0;
			}
			if (p == NULL) break;
		}
		else {
			tok[tl++] = c;
		}
		p = p->next;
	}

	pList out;
	if (calcError == 0 && sp != 1) calcError = 2;
	if (calcError == 0) out = ratToList(&st[0]);
	else { out = makeList(); insertBack(out, makeNode('0')); }

	for (int i = 0; i < sp; i++) ratFree(&st[i]);
	free(st);
	free(tok);
	return out;
}
