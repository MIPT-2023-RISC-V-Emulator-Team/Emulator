int sum(int n) {
	int S = 0;
	for(int i = 0; i < n; i++) {
		S += i;
	}
	return S;
}
int main()
{
	int x = sum(10);
	return 0;
}
