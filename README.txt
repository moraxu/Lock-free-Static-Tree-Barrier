In order to run the test cases:

    1. Place 'CS295' directory in 'model-checker' directory,
    2. 'cd' to that 'CS295' directory,
    3. Issue 'make test1' and 'make test2',
    4. 'cd ..',
    5. Issue './run.sh CS295/test1 -m 2 -y -x 10000' to run test case 1,
    6. Issue './run.sh CS295/test2 -m 2 -y -x 10000' to run test case 2.

Without the '-x 10000' flag the test cases were running for over two hours so I decided to limit the number of
executions to 10000.