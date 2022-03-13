/*
1) bbertol2 1698209 1698093    1 1698209 S echoserveri
2) only 1 and only one thread, looking at the code for echoserveri I believe this is because it is not set up to run multiple treads or process yet
3) the second window recieved it's message back and the server window show a new connection and more bytes recieved, the thrid window remains unchanged
4) 
bbertol2 1699998 1698093    1 1699998 S echoserverp
bbertol2 1700025 1699998    1 1700025 S echoserverp
bbertol2 1700031 1699998    1 1700031 S echoserverp
bbertol2 1700038 1699998    1 1700038 S echoserverp
5) there are now 4 process with three sharing the same pgid, assuming these are the children then it is easy to see that for each incoming connection the parent forks a new child process to handle/work with that connection.
6)
bbertol2 1705670 1698093    4 1705670 S echoservert
bbertol2 1705670 1698093    4 1705683 S echoservert
bbertol2 1705670 1698093    4 1705697 S echoservert
bbertol2 1705670 1698093    4 1705707 S echoservert
7) There are now 4 threads with only one process, we can tell its only one process thanks to the pid. I am assuming this is because this server is set up to start a new thread for each connection.
8)
bbertol2 1706112 1698093    9 1706112 S echoservert_pre
bbertol2 1706112 1698093    9 1706113 S echoservert_pre
bbertol2 1706112 1698093    9 1706114 S echoservert_pre
bbertol2 1706112 1698093    9 1706115 S echoservert_pre
bbertol2 1706112 1698093    9 1706116 S echoservert_pre
bbertol2 1706112 1698093    9 1706117 S echoservert_pre
bbertol2 1706112 1698093    9 1706118 S echoservert_pre
bbertol2 1706112 1698093    9 1706119 S echoservert_pre
bbertol2 1706112 1698093    9 1706120 S echoservert_pre
9) With this server there is a total of 9 threads, I think this is because the code creates a fixed number of new worker threads which is 8 and then sends the connection to one of these threads
10) I see 8 before wait (items) so I believe there are 0 producer threads
11) there are 8 for reasons stated previously
12) while there are no print statments, I assume they are waiting on a connection
13) they are waiting on items
14) the connection, since there are open slots it is not waiting on those
15) a producer posting an item
16) one
17) a producer posting an item
18) there are no statements for waiting on a slot so I assume another connection or something to put into a slot.
19) echoserverp, echoservert, echoservert_pre
20) echoservert_pre, I imagine p and t both have theoritical limits based off of hardware, but pre appears to be limited to 8.
21) pre, it utilizes the producer consumer model enablining it to share items
22) p, or t seem both easy to implement while pre seems more intimidating.
*/
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

int main (void) {
    int MAX = 500;
    char* query;
    char content[MAX];
    
    query = getenv("QUERY_STRING");
    
    sprintf(content, "The query string is: %s", query);
    printf("Content-type: text/plain\r\n");
    printf("Content-length: %d\r\n\r\n", (int)strlen(content));

    printf("%s", content);
    fflush(stdout);

    return 0;
}