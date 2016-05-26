# Ideas

## tsss (transparent shared-memory-socket-swapping)

### Basic Connection Workflow

We have a client and a server. The server has a domain socket open, on which it
waits for any incoming requests-to-connect from any client. We will need such a
domain socket, because there must first be a request to the server by a client
before any shared memory can be allocated. As soon as a server receives a
request-to-connect on its primary socket, the standard socket API gives the
server a unique socket over which to communicate with this specific client. The
server will then allocate a new shared-memory segment, associated with a segment
ID. This ID must be transferred to the client so that it can attach the memory
segment to its address space. Note that we do not make use of the segment key
(via which the server and client could get the segment independently) after
creating the segment. To transfer the ID, we use the domain
socket. Theoretically, we could then close the domain socket for this connection
and work only with the shared memory, but we might as well keep the domain
socket connection for exchange of meta-information, exchange of larger messages
(for which the allocated shared memory does not suffice) or exchange of other
control-signals. Most importantly, the client (or server) will want to
terminate the connection at one point. This would best be done via the socket.

### Shared Memory Exchange

The shared memory shall be allocated to contain two unidirectional *channels*,
one which is written to by the client and read by the server, and the other one
for the opposite direction. These channels will be implemented as circular
buffers (queues), with a read and a write pointer/index. The only drawback of
circular buffers is usually the necessity to distinguish between an empty and a
full buffer, as the write pointer will equal the read pointer in *both* cases
for most implementations. There are many ways to handle this:

1. Keep a size counter to handle discrimination of empty and full buffers. This
   requires an additional integer.
2. Make the pointers go around the buffer twice, checking the first
   bit. Requires no additional integer and in fact less logic and computation.
   https://gist.github.com/engelmarkus/a06c4ad432e1ef2ba6b134f9bb5cd06a
3. Exploit alignment constraints on pointers and use the first bit of the write
   (or read) pointer as a flag.

Because the available input space is possibly non-contiguous, messages will
sometimes have to be written by the implementation with two calls to
`memcpy`. Given that the read and write functions return the number of bytes
read or written, we can use these return values to indicate that the buffer is
full (returning zero for an attempt to write more data than there was space
available) or empty. We would most likely not want partial reads or writes,
i.e. returning a value $0 < \text{returned value} < \text{size}$.

### Step-By-Step Example

1. The server creates a new domain socket with a call to `socket`.
2. The server binds the socket to an address with `bind`.
3. The server begins to listen on the socket with `listen`.
4. The server begins to accept connections on the socket with `accept`.
5. The client creates a new domain socket with `socket`.
6. The client connects to the server's primary domain socket with `connect`.

Excluding the client's call to `connect` and the server's call to `accept`, all
functions mentioned above are those from the standard socket API, i.e. they
require no new implementation. `connect` and `accept` will need to handle
creation of the shared memory.

As soon as the server accepts an incoming connection (originating from a
client's call to `connect`), it is given a new socket to communicate uniquely
with this specific client. This is the socket we will use for exchange of
meta-information, control-signals, large messages etc. Once we have this socket,
we can allocate a shared memory segment containing our two buffers. This segment
will be associated with an ID, which must be communicated to the client. For
this, we use the socket just created, calling `write` or `send`. On the client
side, inside `connect`, we first also call the standard `connect` function from
the socket API to get the communication socket file descriptor. After, we
immediately `read`/`recv` on that socket to get the segment ID. At this point,
both the server and the client will have the segment IDs. Both can now attach it
to their address space and communication over them can begin.

Note that `accept` returns a socket file descriptor in the socket API, i.e. a
single identifier for the connection. Also on the client side, there is only the
socket FD returned by `socket` to identify the connection. However, we really
actually have two identifiers: the file descriptor for the socket and the
segment ID. Also, the call to `shmat` yields a pointer to the shared memory,
which also needs to be stored somewhere. As such, we need some way of
associating a unique identifier with all three of these values, such that calls
to `read`, `write`, `close` etc. can all work with the *single* value returned
by `accept` and `connect` (which would normally just be the socket file
descriptor). One possibility would be to maintain a global mapping from the
socket file descriptor to the memory ID and the memory segment. In calls to
`read`, `write` etc. we could then retrieve the segment ID and shared memory as
necessary. Note that we have to use the socket as a key because that is the only
identifier the client ever gets (when calling `socket`), as opposed to the
server which gets a new identifier for each connection (which we could have
chosen as the segment ID). Note also that on the client-side, we theoretically
don't need to store the segment ID, as it is only required by one party to
deallocate the segment.

```C
int accept(primary_socket, ...) {
	// Call the socket API's accept
	socket = accept(primary_socket, ...);

	// Create a shared memory segment
	segment_id = shmget(key, size, ...);

	// Initialize our data structures

	// Communicate the ID with the client
	send(connection, segment_id);

	// Attach the segment to our address space
	memory = shmat(segment_id);

	// Map the socket to its associated segment
	// ID and shared memory pointer
	map(socket, segment_id, memory);

	return segment_id;
}
```

```C
void connect(connection, server_address) {
	int segment_id;

	// Socket API call
	connect(connection, server_address, ...);

	// Get the segment ID
	read(connection, &segment_id, sizeof segment_id);

	// Attach the memory segment
	memory = shmat(segment_id, ...);

	map(connection, segment_id, memory);
}
```

Note: we also need to construct the data structures in the shared memory
(e.g. in map).
Note: Also need to handle the fact that each end may only read or write from/to
its respective buffer. Will need a `connection_info.who` identifier anyway.

How to wrap syscalls?
http://stackoverflow.com/questions/3662856/how-to-reimplement-or-wrap-a-syscall-function-in-linux
<3

Note that because we will only override `read`/`write` or `recv`/`send` (only
one of these two pairs), the other will remain for exclusive communication over
the domain socket.

One idea would actually be to allocate two shared memory segements, one for each
channel. Then we could set the file permissions on server/client side such that
each side could only read or write to the buffers it is allowed to.

```C
struct Connection* connection(int socket_fd) {
	return connection_map[socket_fd];
}

// More like: read_buffer(int socket_fd)

void create_buffer(RingBuffer** buffer, int size, int segment_id) {

	// Get the memory
	*buffer->memory = shmat(segment_id);

	//Assign the ID
	*buffer->id = segment_id;

	// Setup the pointers
	*buffer->read = *buffer->write = *buffer->memory;

	// Setup the integers
	*buffer->capacity = size;
	*buffer->size = 0;
}

void accept(primary_socket, ...) {
	socket = accept(primary_socket, ...);

	first_segment = shmget(key, size, flags);
	send(socket, &first_segment, sizeof first_segment);
	create_buffer(&connection(socket).read_buffer, first_segment);

	second_segment = shmget(key, size, flags);
	send(socket, &second_segment, sizeof second_segment);
	create_buffer(&connection(socket).write_buffer, second_segment);

	return socket;
}
```

## TODO

1. Write domain socket code
2. Write buffer code
3. Write connection-info struct code
4. Override `connect` and `accept`
5. Override `send` and `recv`
6. Profit $
