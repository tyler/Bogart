Bogart
------

Bogart is a (mostly facetious) response to the Sinatra project.  The primary difference
between the two projects is that Bogart is written in C, with the purpose of making it quick
and easy to write little web services in C.

This code has every security flaw you imagine, tons of memory leaks, and when I wrote it I
may have been awake for much longer than one should be when writing C.

A secondary reason for its existence was to give me an excuse to play with Apple's anonymous
function extension to C. So, given that, it will only compile on OS X.

It uses libevent, so you'll need that.

If I get around to finishing it, it'll also support simple little models using Redis.


Example
-------

    #include <stdio.h>
    #include "bogart.h"
    
    Bogart {
        UseRedis;
        
        get("/hello") {
            body("<h1>Hello, World!</h1>");
        };
    
        get("/create") {
            redisCommand(_redisFd, "HSET User:%s %s %s", params("id"), "name", params("name"));
            body("User created.");
        };
    
        get("/show") {
            redisReply * reply = redisCommand(_redisFd, "HGET User:%s %s", params("id"), "name");
            view("index.cml", map("name", reply->reply));
        };
    
        Start(11000)
    }


Pretty stupid simple.

You might notice that "index.cml" bit in the last route. I rolled a crappy little template
system into it. You pass `view` a filename and a mapping of keys to strings.


So, check it out if you're curious how it's done... But don't use it for anything. Ever.


[Tyler McMullen](http://drmcawesome.com/)
