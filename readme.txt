Hang Ruan
V00923058
To compile: run "make"
TO run the code: run "./ACS customers.txt"



1. Program simulates airline check_in system with customers come in at different time and requires various serving time. It outputs when customers enter queue, start being served and finish being served.

2. Program sometimes run into deadlock. Its due to when the first clerk gets a customer from queue, then second clerk gets another customer from the same queue, but instead of getting the second customer ID it gets the first customer ID. I tried mutex and semaphor and was unable to fix the problem.
