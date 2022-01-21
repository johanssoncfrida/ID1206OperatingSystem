# OperatingSystems - ID1206

## Sem 1 - Boot Kernel

In this tutorial you’re going to boot your own computer (or virtual machine)
from scratch. Your program will hopefully be able to print “OK” on the
screen, so it’s probably the most meaningless program that you have ever
written; you will however be very proud and live your life with the comfort
of knowing that there is no magic to operating systems.

## Sem 2 - Boote Kernel

Similar to Sem 2 but print name

## Sem 3 - Malloc and free
Implement your own malloc() and free(). 
This is an assignment where you will implement your own malloc using a
scheme similar to dlmalloc, Doug Lee's malloc. You should be familiar with
the role of the allocator and how to implement and benchmark a simple
version of it.. Write a report, in Latex, according to directives. 

## Sem 4 - Green Threads
This is an assignment where you will implement your own thread library.
Instead of using the operating systems threads you will create your own
scheduler and context handler. Before even starting to read this you should
be up and running using regular threads, spin locks, conditional variables,
monitors etc. You should also preferably have done a smaller exercise that
shows you how we can work with contexts.
Note - the things you will do in this assignment are nothing that you
would do in real life. Whenever you want to implement a multi-threaded
program you would use the pthread library. We will however manage con-
texts explicitly to implement something that behaves similar to the pthread
library. Why? - To better understand how threads work on the inside.
We will call our implementation green since they will be implemented in
user space i.e. the operating system will not be involved in the scheduling
of threads (it does perform the context switching for us so the threads are
not all green)
