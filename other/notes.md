# Notes

## The Forking Problem (Multi-Process Support)

Question: Must we handle `fork()` calls? If so, how?  Answer:

When a process `fork()`s another process, it creates a child process. The
process that called `fork()` then becomes known as the parent process. Two
things are especially interesting for us: how the parent and child relate with
respect to file descriptors, and how they relate with respect to their memory
(address) space and especially shared memory within that memory space:

* __File descriptors__: File descriptors are *shared* between related
  processes. That is, the same file descriptor (i.e. the same magic integer)
  will refer to the same underlying object (socket, file, pipe) for both the
  parent and the child. Reads and writes will thus read from or modify the same
  entity.

* __Address Space__: When a process creates a child process, the child's address
  space is an exact *replication* of its parent. This means that any variables
  or data available to the parent process will also be available to the
  child. However, this does not mean that the parent and child *share* this
  address space. Any modifications to the data by the parent will, after
  forking, be invisible to the child and vice-versa. For example, if the parent
  has a variable `x`, then after forking both the parent and the child will have
  access to the value of `x` before forking. However, when the child modifies
  `x`, this will not modify the `x` of the parent, because the child received a
  copy of x, not a reference to it. In detail, a *copy-on-write* mechanism is
  implemented. This means that initially, the child's virtual address space does
  indeed refer to the same pages in memory as the virtual addresses of the
  parent. However, as soon as either the parent or the child try to write a
  page, a corresponding copy of that page will be made for the child, so that
  the changes are invisible to the other party.

* __Shared Memory__: As it was determined empirically, the above rule that
  memory is not shared between related processes is not valid for shared memory
  (or memory mapped files). I.e., shared memory segments will indeed be shared
  between related
  processes. [This](http://stackoverflow.com/questions/13274786/how-to-share-memory-between-process-fork)
  links indicates that this really is the case.

What this last point basically means is that we do not have to do anything at
all to support forking. When the server calls `accept()` and receives an
incoming connection, it will create the shared memory segment with the read and
write buffers in it. When it calls `fork()` thereafter, that child process will
have access to that same buffer, so that reads and writes between that child
process and the source (client) process can function perfectly normally.

What becomes a bigger problem is that when a process `fork`s a child and then
closes the connection socket (because it would normally no longer need it), the
current setup would destroy (deallocate) the entire memory segment because it is
assumed that the server process has ownership over the segment (right now, we
only close at the end of a communication process, when all parties are
finished). This means that the child process will no longer be able to
communicate over the connection buffer.

The solution to this is reference counting. Basically, next to the two buffers,
we will also store a count of processes using the segment in the shared memory
segment. This count will be incremented on two occasions:

* Inside `setup_connection`, which is called when either the server or the
  client open the connection in `accept` or `connect`, respectively.
* Inside `fork()`. The thing is that, as it was explained above, child processes
  inherit all file descriptors from their parent. This means that any correct
  program would need to close those descriptors from within the parent *and from
  within the child*, as the two descriptors are independent from each other
  (i.e. one can close their descriptor without closing/removing the underlying
  object, meaning it would still be open for the other process). So in one way,
  all underlying objects are actually reference counted. Or at least, any object
  in the kernel that needs automatic (as in, not done by the user)
  destruction/deallocation at one point would need to keep track of how many
  processes `open()`-ed that file descriptor and how many `close()`-ed it. When
  all processes have closed their descriptor, the underlying object could be
  deallocated.

So that is what we should do too. We put an *atomic* reference count into shared
memory and increment it when first attaching the segment and then also in each
`fork()`. So basically, whenever we `fork()` we would need to iterate over all
open connection objects (meaning whatever data structure we use should allow
iteration) and increment their reference count. Whenever a process then calls
`close()`, we call our `disconnect()` function internally, which right now just
detaches the segment from the address space. Additionally, we'll now also
decrement the reference count and destroy the segment when it reaches zero.

## Lookup Tables

Problem: Hashtables are not efficient enough.

Solution 1:

Assume (socket) file descriptors are always small, such that we can simply
allocate a fixed-size array rather than a hashtable to do our lookups. My
research showed that file descriptors in all but rare cases will be in a range
from 0 to 1023, so using an array of that size should be OK, at least in terms
of having enough space. One thing that is for sure is that the kernel does not
just generate random values in the whole 32-bit integer range. Rather, the
descriptors will be small (possibly the smallest available) and will be
recycled. So what the maximum FD value really depends on is the number of open
files allowed in a process. This limit is controlled in
`/etc/security/limits.conf` and can be viewed via `ulimit -n`. On my system it
seems to be 7168, on the server 65536.

Another problem with this approach may be too much wasted memory. Taking the
pointer to the reference count into account, we end up with three pointers (the
other two for the buffer pointers) and an integer for the segment ID, per
connection object. That gives $3 \cdot 8 + 4 = 28$ Bytes per object. If we
allocate an array of 1024 file descriptors and only use, say, 24, we end up with
1000 useless connection objects, totalling 28 KB of wasted memory.

http://serverfault.com/questions/165316/how-to-configure-linux-file-descriptor-limit-with-fs-file-max-and-ulimit
http://stackoverflow.com/questions/8059616/whats-the-range-of-file-descriptors-on-64-bit-linux
http://stackoverflow.com/questions/18507057/what-are-the-possible-values-for-file-descriptors

Solution 2:

The second option would be to generate our own connection keys and store the
socket FD in the connection object (not in the shared memory, just in the
object). Again this has advantages and disadvantages:

* Advantages: Effectively solves the storage problem, as we can simply return
  incrementing keys, from 0, and index into an array. It would probably be best
  to implement a (light-weight) resizable array/vector data structure for this,
  starting with some initial size of say, 32 connections, and then doubling from
  there on (note: Facebook's C++ library actually resizes by a factor of 1.5
  rather than 2, but who cares).

* Disadvantages: The primary disadvantage would be that we're no longer
  returning the socket FD. This boils down to having to wrap the entire Socket
  API because any calls to socket syscalls in the target program would obviously
  expect the socket FD (to the domain socket we keep open next to the buffer)
  and not our key. However, from what I see there aren't *that* many socket
  functions in the standard API anyway [1]. Also, we'd probably want to wrap and
  re-implement those functions anyway at some point (select, poll etc.), so this
  might not be a big problem at all.

[1] https://en.wikipedia.org/wiki/Berkeley_sockets#Socket_API_functions

## Socket-Specific Selection

Question: Is it possible to have only some sockets returned from `accept()` be
TSSX connecitions, and others just standard sockets with no associated shared
memory? If so, how?

Solution:

The solution consists of three topics of discussion: how to select sockets to
use TSSX in general, how to do it in code and, lastly, once it works, how to
distinguish between standard sockets and TSSX connections in syscalls like
`read` or `write`.

1. The general idea is to select which domain sockets use TSSX by specifying
   their socket paths in an environment variable. The socket path is the only
   thing we have to identify a socket before executing a process, as it must be
   known beforehand. For example, postgres seems to always open the
   `/var/run/nscd/socket` socket. Once we know what socket paths should use
   TSSX, we can specify them in a space-separate string as an environment
   variable `USE_TSSX` and parse them in the library.

2. In code, we will need to maintain two things: a flag whether or not we have
   fetched and parsed the environment variable yet (for the first call) and,
   secondly, a lookup table (hash-set, or array when few strings) for the
   strings themselves. Inside `accept` and `connect`, we would then always first
   check if we have fetched the environment variable yet. If not, we fetch it
   with `getenv` and parse it by splitting the string and putting the socket
   paths in a lookup table. Once we have initialized that lookup table, we can
   then always see if the socket path in the calls to `accept` and `connect` are
   in the list of socket paths that should use TSSX. If so, we allocate the
   segment, setup the connection etc. If not, we just return the result of the
   wrapped `accept` call with the socket ID.

3. Once we have the distinction between domain sockets and sockets using TSSX,
   we must handle that distinction in *all calls* such as `write`, `select` or
   `close`. In the simplest case, we would maintain a hash-set for FDs of
   sockets using TSSX and then always see if the FD for a syscall is in that
   set. However, this could quickly become expensive. For this reason, it might be better to actually encode the
   "using TSSX" property in the generated ID value itself (assuming we generate
   our own IDs). For example, rather than return positive IDs that would provide
   no basis for distinguishing them from domain socket FDs (not using TSSX), we
   could in fact generate *negative IDs* (excluding small negative values like
   -1 as error codes, so maybe something like -100 onwards). The "lookup" to see
   if a socket uses TSSX or not would simply mean checking if the value is negative (TSSX) or positive (domain socket) and would thus literally be free (a single comparison instruction). Talk about $O(1)$. To turn the negative ID into an array index (for connection objects), we would just negate and offset it.
