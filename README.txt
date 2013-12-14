sws(1)

Authors
Jordan Scales, Nick Smith, Patrick Cedrowski, Paul-Anthony Dudzinski

README
sws is a simple web server designed to implement HTTP 1.0 requests for the 
final project of CS631 APUE. The webserver currently implements GET and 
HEAD requests and returns the appropriate status code based on RFC1945. In
the future sws will implement other features to be assigned in future project
milestones. 

HISTORY
According to the manual for sws(1):
    "A simple http server has been a frequent final project assignment for the
     class 'Advanced Programming in the UNIX Environment' as Stevens Institute
     of Technology. This variation was first assigned in the Fall 2008 by Jan
     Schaurmann."

INSTALL
Copy the files of sws to the desired directory and run the make(1) command.

BUILD ARTIFACTS
sws: (make sws)
The server executable. Run with -h for usage.

unittest: (make test)
Executes unit tests on the parser directly, based on the tests specified
in test.in. Output is written to test.out and test results are written
to stdout.

COPYRIGHT (c) 2013

BUGS
None we hope.
