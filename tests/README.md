# slate tests

To run the quick and non-interactive tests which runs the test programs
that begin with a three digit number:
```sh
  $ ./run_tests
```

Valgrind tests require valgrind to be installed in your PATH.  Running the
quick tests and non-interactive with valgrind will take a lot more time.
To run the same non-interactive tests from above with valgrind run:
```sh
  $ ./valgrind_run_tests
```

Tests from files that begin with an underscore are interactive tests.

Test program files that do not start with a three digit number or an
underscore are some kind of developer's test, that is, codes that are
experiments in figuring out how to do stuff.

