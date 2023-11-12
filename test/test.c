// ~O(n^3)
int sum(int n) {
	int S = 0;
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < n; j++) {
			for(int k = 0; k < n; k++) {
				S += i + j + k;
			}
		}
	}
	return S;
}
int main()
{
	int x = sum(150);
	return 0;
}
