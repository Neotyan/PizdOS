int sprint(unsigned int num, char *buffer) {
	int index = 0;

	if (num < 10) {
		buffer[index++] = '0';
		buffer[index++] = num + '0';
		buffer[index] = '\0';
		return 2;
	}

	do {
		int tmp = num % 10;
		buffer[index++] = tmp + '0';
		num /= 10; 
	} while (num > 0);

	int start = 0;
	int end = index - 1;

	while (start < end) {
		char swap_tmp = buffer[start];
		buffer[start] = buffer[end];
		buffer[end] = swap_tmp;
		start++;
		end--;
	}

	 buffer[index] = '\0';
	 return index;
}