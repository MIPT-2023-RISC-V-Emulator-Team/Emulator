
int main() {
	int a = 0, b = 1;
	for (int i = 0; i < 20; ++i) {
		int tmp = b;
		b += a;
		a = tmp;
	}
	return b;
}
