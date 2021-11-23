struct symbol
{
	uint8_t vals[5];
};

symbol* pick_symbol_with_AI()
{
	symbol* s = calloc(0x5, 0x1);
	int r = rand() % X;
	for(int i = 0; i < 5; ++i)
	{
		if(i > r)
		{
			s[i] = 0;
		}
		else
		{
			s[i] = rand_() % 26 + 65;
		}
	}

	return s;
}

uint8_t matrix[0x38] = {
	1, 0, 0, 0, 
	1, 1, 1, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	0, 0, 0, 1, 
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	1, 0, 1, 1, 
	1, 1, 1, 0, 
	1, 0, 0, 0, 
	0, 0, 1, 1, 
	0, 0, 1, 0, 
	1, 0, 0, 1
}

void buy_stonks()
{
	symbol* symbols[8]; //RBP-0x220h
	for(int i = 0; i < 8; ++i)
	{
		symbols[i] = pick_symbol_with_AI();
	}

	puts("What symbol do you want to see?");
	int stockId; //RBP-0x224
	sscanf("%d", &stockId);
	symbol* s = symbols[stockId];
	puts(s);
	puts("What is your API token?");
	getchar();
	char user_buf[0x130];
	read(STDIN, user_buf, 0x12C);
	int leaked = 0; // RBP-0x8
	for(int i = 0; i < 0x12C; ++i)
	{
		int8_t b = user_buf[i];
		int32_t c = b; //movsx
		if((c - 0x41) <= 0x37)
		{
			if(matrix[c])
			{
				//leak
				if(leaked >= 0)
				{
					puts("Hey, only one leak allowed!");
					exit(0);
				}

				if(c != 0x63)
				{
					puts("Hey, only one leak allowed!");
					exit(0);
				}

				leaked += 1;
			}
		}
	}

	puts("Buying stonks with token:");
	printf(user_buf);
	putchar('\n');
	return 0;
}

int main()
{
	setbuf(STDOUT, NULL);
	setbuf(STDERR, NULL);
	srand(time(0));
	puts("Welcome back to the trading app!");
	puts("What would you like to do?");
	puts("1) Buy some stonks!");
	int choice;
	sscanf("%d", &choice);
	if(choice == 1)
	{
		buy_stonks();
	}

	return 0;
}
