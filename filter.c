/*
  Course: CS 30200
  Name: Jacob Yang
  Assignment: 1

*/
#include <stdio.h>   // scanf(), fscanf(), fprintf(), fopen()
#include <stdlib.h>  // atoi(), getenv()

int main(int argc, char* argv[])
{
	int columns = 0;
	int decimals = 0;

	FILE* fp;
	if ((fp = fopen("filter.cfg", "r")) != NULL)
	{
		//first value = decimal places, second value = number of columns
		fscanf(fp, "%d", &decimals);
		fscanf(fp, "%d", &columns);
	}

	// Override the default value with an environment variable value.
	char* num;
	if ((num = getenv("CS302_COLUMNS")) != NULL)
	{
		//get number of columns from the environment
		columns = atoi(num);
	}
	if ((num = getenv("CS302_PRECISION")) != NULL)
	{
		//get how many decimal places after the point from the environment
		decimals = atoi(num);
	}

	// Get a command line argument (if it exists).
	if (argc > 1)
	{  // get an operand from the command line
		columns = atoi(argv[1]);
		if (argv[2] != 0)
			decimals = atoi(argv[2]);
	}

	// Process the stream of input numbers.
	double x;
	int count = 0;
	while (scanf("%lf", &x) != EOF)
	{
		if (decimals > 0) //Runs this when there is a decimal value present
		{
			printf("%10.*f  ",decimals, (x));
		}
		else //Runs this when decimals value is 0 or the default parameters
		{
			printf("%20.13f",(x));
		}
		count++;
		if (count == columns || ((columns == 0 && count == 3)))
		{
			printf("\n");
			count = 0;
		}
	}

	return 0;
}

